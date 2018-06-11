#include "common.h"
#include "../src/utils/crypto.h"
#include "../src/utils/system.h"
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio.hpp>

Test::Test() { SetupLogging(YOGI_VB_TRACE); }

Test::~Test() {
  EXPECT_EQ(YOGI_DestroyAll(), YOGI_OK);

  YOGI_LogToConsole(YOGI_VB_NONE, 0, 0, nullptr, nullptr);
  YOGI_LogToHook(YOGI_VB_NONE, nullptr, nullptr);
  YOGI_LogToFile(YOGI_VB_NONE, nullptr, nullptr, 0, nullptr, nullptr);
}

BranchEventRecorder::BranchEventRecorder(void* context, void* branch)
    : context_(context), branch_(branch), json_str_(10000) {
  StartAwaitEvent();
}

nlohmann::json BranchEventRecorder::RunContextUntil(
    int event, const boost::uuids::uuid& uuid, int ev_res) {
  while (true) {
    for (auto& entry : events_) {
      if (entry.uuid == uuid && entry.event == event &&
          entry.ev_res == ev_res) {
        return entry.json;
      }
    }

    auto n = events_.size();
    while (n == events_.size()) {
      int res = YOGI_ContextRunOne(context_, nullptr, -1);
      EXPECT_EQ(res, YOGI_OK);
    }
  }
}

nlohmann::json BranchEventRecorder::RunContextUntil(int event, void* branch,
                                                    int ev_res) {
  return RunContextUntil(event, GetBranchUuid(branch), ev_res);
}

void BranchEventRecorder::StartAwaitEvent() {
  int res = YOGI_BranchAwaitEvent(branch_, 0, &uuid_, json_str_.data(),
                                  static_cast<int>(json_str_.size()),
                                  &BranchEventRecorder::Callback, this);
  EXPECT_EQ(res, YOGI_OK);
}

void BranchEventRecorder::Callback(int res, int event, int ev_res,
                                   void* userarg) {
  if (res == YOGI_ERR_CANCELED) return;

  auto self = static_cast<BranchEventRecorder*>(userarg);
  self->events_.push_back({self->uuid_,
                           nlohmann::json::parse(self->json_str_.data()), event,
                           ev_res});

  self->StartAwaitEvent();
}

FakeBranch::FakeBranch()
    : udp_ep_(boost::asio::ip::make_address(kAdvAddress), kAdvPort),
      udp_socket_(ioc_, kUdpProtocol),
      acceptor_(ioc_),
      tcp_socket_(ioc_) {
  acceptor_.open(kTcpProtocol);
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(boost::asio::ip::tcp::endpoint(kTcpProtocol, 0));
  acceptor_.listen();
  info_ = objects::detail::BranchInfo::CreateLocal(
      "Fake Branch", "", utils::GetHostname(), "/Fake Branch",
      acceptor_.local_endpoint(), 1s, 1s);

  udp_socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
  udp_socket_.bind(boost::asio::ip::udp::endpoint(kUdpProtocol, 0));
  udp_socket_.set_option(boost::asio::ip::multicast::join_group(
      boost::asio::ip::make_address(kAdvAddress)));
}

void FakeBranch::Connect(void* branch,
                         std::function<void(utils::ByteVector*)> msg_changer) {
  tcp_socket_.connect(GetBranchTcpEndpoint(branch));
  Authenticate(msg_changer);
}

void FakeBranch::Accept(std::function<void(utils::ByteVector*)> msg_changer) {
  tcp_socket_ = acceptor_.accept();
  Authenticate(msg_changer);
}

void FakeBranch::Disconnect() {
  tcp_socket_.shutdown(tcp_socket_.shutdown_both);
  tcp_socket_.close();
}

void FakeBranch::Advertise(
    std::function<void(utils::ByteVector*)> msg_changer) {
  auto msg = *info_->MakeAdvertisingMessage();
  if (msg_changer) msg_changer(&msg);
  udp_socket_.send_to(boost::asio::buffer(msg), udp_ep_);
}

bool FakeBranch::IsConnectedTo(void* branch) const {
  struct Data {
    boost::uuids::uuid my_uuid;
    boost::uuids::uuid uuid;
    bool connected = false;
  } data;

  data.my_uuid = info_->GetUuid();

  int res = YOGI_BranchGetConnectedBranches(branch, &data.uuid, nullptr, 0,
                                            [](int, void* userarg) {
                                              auto data =
                                                  static_cast<Data*>(userarg);
                                              if (data->uuid == data->my_uuid) {
                                                data->connected = true;
                                              }
                                            },
                                            &data);
  EXPECT_EQ(res, YOGI_OK);

  return data.connected;
}

void FakeBranch::Authenticate(
    std::function<void(utils::ByteVector*)> msg_changer) {
  // Send branch info
  auto info_msg = *info_->MakeInfoMessage();
  if (msg_changer) msg_changer(&info_msg);
  boost::asio::write(tcp_socket_, boost::asio::buffer(info_msg));

  // Receive branch info
  auto buffer =
      utils::ByteVector(objects::detail::BranchInfo::kInfoMessageHeaderSize);
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));
  std::size_t body_size;
  objects::detail::BranchInfo::DeserializeInfoMessageBodySize(&body_size,
                                                              buffer);
  buffer.resize(body_size);
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));

  // ACK
  ExchangeAck();

  // Send challenge
  auto my_challenge = utils::GenerateRandomBytes(8);
  boost::asio::write(tcp_socket_, boost::asio::buffer(my_challenge));

  // Receive challenge
  auto remote_challenge = utils::ByteVector(8);
  boost::asio::read(tcp_socket_, boost::asio::buffer(remote_challenge));

  // Send solution
  auto password_hash = utils::MakeSha256({});
  buffer = remote_challenge;
  buffer.insert(buffer.end(), password_hash.begin(), password_hash.end());
  auto remote_solution = utils::MakeSha256(buffer);
  boost::asio::write(tcp_socket_, boost::asio::buffer(remote_solution));

  // Receive Solution
  buffer.resize(remote_solution.size());
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));

  // ACK
  ExchangeAck();
}

void FakeBranch::ExchangeAck() {
  auto buffer = utils::ByteVector({0x55});
  boost::asio::write(tcp_socket_, boost::asio::buffer(buffer));
  boost::asio::read(tcp_socket_, boost::asio::buffer(buffer));
  EXPECT_EQ(buffer[0], 0x55);
}

void SetupLogging(int verbosity) {
  YOGI_LogToConsole(YOGI_VB_TRACE, YOGI_ST_STDERR, YOGI_TRUE, nullptr, nullptr);
  YOGI_LoggerSetComponentsVerbosity("Yogi\\..*", verbosity, nullptr);
}

void* CreateContext() {
  void* context = nullptr;
  int res = YOGI_ContextCreate(&context);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_NE(context, nullptr);
  return context;
}

void RunContextInBackground(void* context) {
  int res = YOGI_ContextRunInBackground(context);
  EXPECT_EQ(res, YOGI_OK);
  res = YOGI_ContextWaitForRunning(context, 1000000000);
  EXPECT_EQ(res, YOGI_OK);
}

void* CreateBranch(void* context, const char* name, const char* net_name,
                   const char* password, const char* path) {
  void* branch = nullptr;
  int res = YOGI_BranchCreate(&branch, context, name, "Description", net_name,
                              password, path, nullptr, kAdvPort,
                              kAdvInterval.count(), kConnTimeout.count());
  EXPECT_EQ(res, YOGI_OK);
  return branch;
}

boost::asio::ip::tcp::endpoint GetBranchTcpEndpoint(void* branch) {
  char json_str[1000] = {0};
  YOGI_BranchGetInfo(branch, nullptr, json_str, sizeof(json_str));

  auto json = nlohmann::json::parse(json_str);
  auto port = json["tcp_server_port"].get<unsigned short>();

  return {boost::asio::ip::address::from_string("::1"), port};
}

boost::uuids::uuid GetBranchUuid(void* branch) {
  boost::uuids::uuid uuid = {0};
  int res = YOGI_BranchGetInfo(branch, &uuid, nullptr, 0);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_NE(uuid, boost::uuids::uuid{});
  return uuid;
}

nlohmann::json GetBranchInfo(void* branch) {
  char str[10000] = {0};
  int res = YOGI_BranchGetInfo(branch, nullptr, str, sizeof(str));
  EXPECT_EQ(res, YOGI_OK);
  return nlohmann::json::parse(str);
}

void CheckJsonElementsAreEqual(const nlohmann::json& a, const nlohmann::json& b,
                               const std::string& key) {
  ASSERT_TRUE(a.count(key)) << "Key \"" << key << "\" does not exist in a";
  ASSERT_TRUE(b.count(key)) << "Key \"" << key << "\" does not exist in b";
  EXPECT_EQ(a[key].dump(), b[key].dump());
}

std::map<boost::uuids::uuid, nlohmann::json> GetConnectedBranches(
    void* branch) {
  struct Data {
    boost::uuids::uuid uuid;
    char json_str[1000];
    std::map<boost::uuids::uuid, nlohmann::json> branches;
  } data;

  int res = YOGI_BranchGetConnectedBranches(
      branch, &data.uuid, data.json_str, sizeof(data.json_str),
      [](int, void* userarg) {
        auto data = static_cast<Data*>(userarg);
        data->branches[data->uuid] = nlohmann::json::parse(data->json_str);
      },
      &data);
  EXPECT_EQ(res, YOGI_OK);

  return data.branches;
}

std::ostream& operator<<(std::ostream& os,
                         const std::chrono::nanoseconds& dur) {
  os << dur.count() << "ns";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const std::chrono::microseconds& dur) {
  os << dur.count() << "us";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const std::chrono::milliseconds& dur) {
  os << dur.count() << "ms";
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::chrono::seconds& dur) {
  os << dur.count() << "s";
  return os;
}

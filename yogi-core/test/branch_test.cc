#include <gtest/gtest.h>
#include <yogi_core.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/asio.hpp>
#include <algorithm>
#include <sstream>
#include <regex>

using namespace std::chrono_literals;

#include "../src/utils/system.h"
#include "../src/api/constants.h"
#include "../src/objects/detail/branch/branch_info.h"
#include "../../3rd_party/json/json.hpp"

const int kAdvPort = 44442;
const char* kAdvAddress = api::kDefaultAdvAddress;
const std::chrono::nanoseconds kAdvInterval = 2ms;
const std::chrono::nanoseconds kConnTimeout = 100ms;
const std::chrono::nanoseconds kBrCleanupInterval = 100ms;

class BranchTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    YOGI_LogToConsole(YOGI_VB_TRACE, YOGI_ST_STDERR, YOGI_TRUE, nullptr,
                      nullptr);
    YOGI_LoggerSetComponentsVerbosity("Yogi\\..*", YOGI_VB_TRACE, nullptr);

    context_ = nullptr;
    int res = YOGI_ContextCreate(&context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(context_, nullptr);

    branch_ = CreateBranch(nullptr, nullptr, nullptr);
    ASSERT_NE(branch_, nullptr);
  }

  virtual void TearDown() override { ASSERT_EQ(YOGI_DestroyAll(), YOGI_OK); }

  void* CreateBranch(const char* name, const char* net_name,
                     const char* password) {
    void* branch;
    auto res =
        YOGI_BranchCreate(&branch, context_, name, nullptr, net_name, password,
                          nullptr, nullptr, kAdvPort, kAdvInterval.count(),
                          kConnTimeout.count(), kBrCleanupInterval.count());
    EXPECT_EQ(res, YOGI_OK);
    return branch;
  }

  void RunContextInBackground() {
    int res = YOGI_ContextRunInBackground(context_);
    ASSERT_EQ(res, YOGI_OK);
    res = YOGI_ContextWaitForRunning(context_, 1000000000);
    ASSERT_EQ(res, YOGI_OK);
  }

  boost::asio::ip::tcp::endpoint GetBranchTcpEndpoint() {
    char json_str[1000] = {0};
    YOGI_BranchGetInfo(branch_, nullptr, json_str, sizeof(json_str));

    auto json = nlohmann::json::parse(json_str);
    auto port = json["tcp_server_port"].get<unsigned short>();

    return {boost::asio::ip::address::from_string("::1"), port};
  }

  std::vector<nlohmann::json> GetDiscoveredBranches() {
    struct Data {
      boost::uuids::uuid uuid;
      char json[10000];
      std::vector<nlohmann::json> branches;
    } data;

    auto res = YOGI_BranchGetDiscoveredBranches(
        branch_, &data.uuid, data.json, sizeof(data.json),
        [](int err, void* userarg) {
          auto data = static_cast<Data*>(userarg);
          data->branches.push_back(nlohmann::json::parse(data->json));
        },
        &data);
    EXPECT_EQ(res, YOGI_OK);

    return data.branches;
  }

  void* context_;
  void* branch_;
};

TEST_F(BranchTest, GetInfoBufferTooSmall) {
  char json[3];
  int res = YOGI_BranchGetInfo(branch_, nullptr, json, sizeof(json));
  EXPECT_EQ(res, YOGI_ERR_BUFFER_TOO_SMALL);
  EXPECT_NE(json[sizeof(json) - 2], '\0');
  EXPECT_EQ(json[sizeof(json) - 1], '\0');
}

TEST_F(BranchTest, GetInfoUuid) {
  boost::uuids::uuid uuid = {0};
  int res = YOGI_BranchGetInfo(branch_, &uuid, nullptr, 0);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_NE(uuid, boost::uuids::uuid());
}

TEST_F(BranchTest, GetInfoJson) {
  boost::uuids::uuid uuid;
  char json_str[1000] = {0};
  int res = YOGI_BranchGetInfo(branch_, &uuid, json_str, sizeof(json_str));
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_NE(std::count(json_str, json_str + sizeof(json_str), '\0'), 0);
  EXPECT_STRNE(json_str, "");
  auto json = nlohmann::json::parse(json_str);

  auto time_regex = std::regex(
      "^20\\d\\d-\\d\\d-\\d\\dT[0-2]\\d:[0-5]\\d:[0-5]\\d\\.\\d\\d\\dZ$");
  auto default_name =
      std::to_string(utils::GetProcessId()) + '@' + utils::GetHostname();

  EXPECT_EQ(json.value("uuid", "NOT FOUND"), boost::uuids::to_string(uuid));
  EXPECT_EQ(json.value("name", "NOT FOUND"), default_name);
  EXPECT_EQ(json.value("description", "NOT FOUND"), "");
  EXPECT_EQ(json.value("net_name", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(json.value("path", "NOT FOUND"), std::string("/") + default_name);
  EXPECT_EQ(json.value("hostname", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(json.value("pid", -1), utils::GetProcessId());
  EXPECT_EQ(json.value("tcp_server_address", "NOT FOUND"), "::");
  EXPECT_GT(json.value("tcp_server_port", 0), 1024);
  EXPECT_TRUE(
      std::regex_match(json.value("start_time", "NOT FOUND"), time_regex));
  EXPECT_EQ(json.value("timeout", -1.0f),
            static_cast<float>(kConnTimeout.count()) / 1e9f);
  EXPECT_EQ(json.value("advertising_address", "NOT FOUND"),
            api::kDefaultAdvAddress);
  EXPECT_EQ(json.value("advertising_port", -1), kAdvPort);
  EXPECT_EQ(json.value("advertising_interval", -1.0f),
            static_cast<float>(kAdvInterval.count()) / 1e9f);
}

TEST_F(BranchTest, Advertising) {
  using namespace boost::asio;
  io_context ioc;
  ip::udp::endpoint ep(ip::make_address("0::0"), kAdvPort);
  ip::udp::socket socket(ioc, ep.protocol());
  socket.set_option(ip::udp::socket::reuse_address(true));
  socket.bind(ep);
  socket.set_option(ip::multicast::join_group(ip::make_address(kAdvAddress)));

  boost::system::error_code error_code;
  std::array<char, 100> data;
  std::size_t data_size = 0;
  ip::udp::endpoint sender_ep;
  bool rx_callback_called = false;

  socket.async_receive_from(buffer(data), sender_ep, [&](auto ec, auto size) {
    error_code = ec;
    data_size = size;
    rx_callback_called = true;
  });

  RunContextInBackground();

  ioc.run_one_for(1s);

  ASSERT_TRUE(rx_callback_called) << "UDP receive callback function not called";
  EXPECT_FALSE(error_code) << error_code.message();
  ASSERT_EQ(data_size, 25) << "Unexpected advertising message size";

  boost::uuids::uuid uuid;
  YOGI_BranchGetInfo(branch_, &uuid, nullptr, 0);

  EXPECT_STREQ(data.data(), "YOGI");
  EXPECT_EQ(data[5], api::kVersionMajor);
  EXPECT_EQ(data[6], api::kVersionMinor);
  EXPECT_EQ(std::memcmp(&uuid, data.data() + 7, 16), 0);
  EXPECT_TRUE(data[23] != 0 || data[24] != 0);
}

TEST_F(BranchTest, Connections) {
  CreateBranch("A", nullptr, nullptr);
  CreateBranch("B", nullptr, nullptr);
  CreateBranch("C", "Other net", nullptr);
  CreateBranch("D", nullptr, "Different password");
  RunContextInBackground();

  auto start_time = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start_time < 1s) {
    auto branches = GetDiscoveredBranches();

    auto num_queried =
        std::count_if(branches.begin(), branches.end(),
                      [](const auto& branch) { return branch.count("name"); });

    auto num_connected = std::count_if(
        branches.begin(), branches.end(),
        [](auto& branch) { return branch.value("connected", false); });

    if (branches.size() >= 4 && num_queried >= 4 && num_connected >= 2) {
      break;
    }
  }

  auto branches = std::map<std::string, nlohmann::json>{};
  for (auto& branch : GetDiscoveredBranches()) {
    branches[branch.value("name", "")] = branch;
  }

  EXPECT_TRUE(branches.count("A"));
  EXPECT_TRUE(branches["A"]["connected"].get<bool>());

  EXPECT_TRUE(branches.count("B"));
  EXPECT_TRUE(branches["B"]["connected"].get<bool>());

  EXPECT_TRUE(branches.count("C"));
  EXPECT_FALSE(branches["C"]["connected"].get<bool>());

  EXPECT_TRUE(branches.count("D"));
  EXPECT_FALSE(branches["D"]["connected"].get<bool>());
}

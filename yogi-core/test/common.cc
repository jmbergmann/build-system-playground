#include "common.h"

Test::Test() { SetupLogging(YOGI_VB_TRACE); }

Test::~Test() {
  EXPECT_EQ(YOGI_DestroyAll(), YOGI_OK);

  YOGI_LogToConsole(YOGI_VB_NONE, 0, 0, nullptr, nullptr);
  YOGI_LogToHook(YOGI_VB_NONE, nullptr, nullptr);
  YOGI_LogToFile(YOGI_VB_NONE, nullptr, nullptr, 0, nullptr, nullptr);
}

BranchEventObserver::BranchEventObserver(void* branch)
    : branch_(branch), json_str_(10000) {
  StartObserve();
}

BranchEventObserver::~BranchEventObserver() {
  int res = YOGI_BranchCancelAwaitEvent(branch_);
  EXPECT_EQ(res, YOGI_OK);
}

void BranchEventObserver::StartObserve() {
  int res = YOGI_BranchAwaitEvent(branch_, 0, &uuid_, json_str_.data(),
                                  json_str_.size(),
                                  &BranchEventObserver::Callback, this);
  EXPECT_EQ(res, YOGI_OK);
}

nlohmann::json BranchEventObserver::Wait(int event,
                                         const boost::uuids::uuid& uuid,
                                         int ev_res) {
  std::unique_lock<std::mutex> lock(mutex_);
  while (true) {
    for (auto& entry : events_) {
      if (entry.uuid == uuid && entry.event == event &&
          entry.ev_res == ev_res) {
        return entry.json;
      }
    }

    auto status = cv_.wait_for(lock, 5s);
    EXPECT_EQ(status, std::cv_status::no_timeout);
  }
}

nlohmann::json BranchEventObserver::Wait(int event, void* branch, int ev_res) {
  return Wait(event, GetBranchUuid(branch), ev_res);
}

void BranchEventObserver::Callback(int res, int event, int ev_res,
                                   void* userarg) {
  if (res == YOGI_ERR_CANCELED) return;
  EXPECT_EQ(res, YOGI_OK);
  auto self = static_cast<BranchEventObserver*>(userarg);

  {
    std::lock_guard<std::mutex> lock(self->mutex_);
    self->events_.push_back({self->uuid_,
                             nlohmann::json::parse(self->json_str_.data()),
                             event, ev_res});
    self->cv_.notify_all();
  }

  self->StartObserve();
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
                   const char* password) {
  void* branch = nullptr;
  int res = YOGI_BranchCreate(&branch, context, name, nullptr, net_name,
                              password, nullptr, nullptr, kAdvPort,
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

#include "common.h"
#include "../../3rd_party/json/json.hpp"

Test::Test() { SetupLogging(YOGI_VB_TRACE); }

Test::~Test() {
  EXPECT_EQ(YOGI_DestroyAll(), YOGI_OK);

  YOGI_LogToConsole(YOGI_VB_NONE, 0, 0, nullptr, nullptr);
  YOGI_LogToHook(YOGI_VB_NONE, nullptr, nullptr);
  YOGI_LogToFile(YOGI_VB_NONE, nullptr, nullptr, 0, nullptr, nullptr);
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

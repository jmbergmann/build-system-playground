#include "../common.h"

#include <boost/uuid/uuid_io.hpp>
#include <regex>

#include "../../src/utils/system.h"
#include "../../../3rd_party/nlohmann/json.hpp"

class BranchTest : public Test {
 protected:
  virtual void SetUp() override {
    context_ = CreateContext();
    branch_ = CreateBranch(context_);
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

TEST_F(BranchTest, GetInfoUuid) { GetBranchUuid(branch_); }

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
  EXPECT_FALSE(json.value("description", "").empty());
  EXPECT_EQ(json.value("net_name", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(json.value("path", "NOT FOUND"), std::string("/") + default_name);
  EXPECT_EQ(json.value("hostname", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(json.value("pid", -1), utils::GetProcessId());
  EXPECT_EQ(json.value("tcp_server_address", "NOT FOUND"), "::");
  EXPECT_GT(json.value("tcp_server_port", 0), 1024);
  EXPECT_TRUE(
      std::regex_match(json.value("start_time", "NOT FOUND"), time_regex));
  EXPECT_EQ(json.value("timeout", -1.0f), kBranchProps["timeout"]);
  EXPECT_EQ(json.value("advertising_address", "NOT FOUND"), kAdvAddress);
  EXPECT_EQ(json.value("advertising_port", -1), static_cast<int>(kAdvPort));
  EXPECT_EQ(json.value("advertising_interval", -1.0f),
            kBranchProps["advertising_interval"]);
}

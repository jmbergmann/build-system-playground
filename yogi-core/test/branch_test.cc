#include <gtest/gtest.h>
#include <yogi_core.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm>
#include <sstream>
#include <regex>

#include "../src/utils/system.h"
#include "../src/api/constants.h"

class BranchTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    context_ = nullptr;
    int res = YOGI_ContextCreate(&context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(context_, nullptr);

    branch_ = nullptr;
    res = YOGI_BranchCreate(&branch_, context_, nullptr, nullptr, nullptr,
                            nullptr, nullptr, nullptr, 0, 0);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(branch_, nullptr);
  }

  virtual void TearDown() override { ASSERT_EQ(YOGI_DestroyAll(), YOGI_OK); }

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
  char json[1000] = {0};
  int res = YOGI_BranchGetInfo(branch_, &uuid, json, sizeof(json));
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_NE(std::count(json, json + sizeof(json), '\0'), 0);
  EXPECT_STRNE(json, "");

  std::stringstream ss;
  ss << json;
  boost::property_tree::ptree pt;
  boost::property_tree::read_json(ss, pt);

  auto time_regex = std::regex(
      "^20\\d\\d-\\d\\d-\\d\\dT[0-2]\\d:[0-5]\\d:[0-5]\\d\\.\\d\\d\\dZ$");
  auto default_name =
      std::to_string(utils::GetPid()) + '@' + utils::GetHostname();

  EXPECT_EQ(pt.get("uuid", "NOT FOUND"), boost::uuids::to_string(uuid));
  EXPECT_EQ(pt.get("name", "NOT FOUND"), default_name);
  EXPECT_EQ(pt.get("description", "NOT FOUND"), "");
  EXPECT_EQ(pt.get("net_name", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(pt.get("path", "NOT FOUND"), std::string("/") + default_name);
  EXPECT_EQ(pt.get("hostname", "NOT FOUND"), utils::GetHostname());
  EXPECT_EQ(pt.get("pid", -1), utils::GetPid());
  EXPECT_EQ(pt.get("advertising_address", "NOT FOUND"), api::kDefaultAdvAddress);
  EXPECT_EQ(pt.get("advertising_port", -1), api::kDefaultAdvPort);
  EXPECT_EQ(pt.get("advertising_interval", -1),
            (float)api::kDefaultAdvInterval / 1000.0f);
  EXPECT_TRUE(std::regex_match(pt.get("start_time", "NOT FOUND"), time_regex));
  EXPECT_EQ(pt.get("active_connections", -1), 0);
}

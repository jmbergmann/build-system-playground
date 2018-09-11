/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"
#include "../../src/utils/system.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <regex>

class BranchTest : public Test {
 protected:
  virtual void SetUp() override {
    context_ = CreateContext();
    branch_ = CreateBranch(context_);
  }

  void* context_;
  void* branch_;
};

TEST_F(BranchTest, CreateWithDefaults) {
  void* branch;
  int res = YOGI_BranchCreate(&branch, context_, nullptr, nullptr, nullptr, 0);
  EXPECT_EQ(YOGI_OK, res);
}

TEST_F(BranchTest, CreateWithJsonPointer) {
  nlohmann::json props;
  props["arr"] = {"some string", kBranchProps, 123};
  props["arr"][1]["name"] = "Samosa";

  char err[100];

  void* branch;
  int res = YOGI_BranchCreate(&branch, context_, props.dump().c_str(),
                              "/blabla", err, sizeof(err));
  EXPECT_EQ(YOGI_ERR_PARSING_JSON_FAILED, res);
  EXPECT_STRNE(err, "");

  res = YOGI_BranchCreate(&branch, context_, props.dump().c_str(), "/arr/1",
                          err, sizeof(err));
  EXPECT_EQ(YOGI_OK, res);
  EXPECT_STREQ(err, "");
  auto info = GetBranchInfo(branch);
  EXPECT_EQ(info.value("name", "NOT FOUND"), "Samosa");
}

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
  EXPECT_EQ(json.value("network_name", "NOT FOUND"), utils::GetHostname());
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
            static_cast<float>(kBranchProps["advertising_interval"]));
  EXPECT_EQ(json.value("ghost_mode", true), false);
}

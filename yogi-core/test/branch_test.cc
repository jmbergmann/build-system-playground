#include <gtest/gtest.h>
#include <yogi_core.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio.hpp>
#include <algorithm>
#include <sstream>
#include <regex>

using namespace std::chrono_literals;

#include "../src/utils/system.h"
#include "../src/api/constants.h"

const auto kAdvInterval = 2ms;

class BranchTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    context_ = nullptr;
    int res = YOGI_ContextCreate(&context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(context_, nullptr);

    branch_ = nullptr;
    res = YOGI_BranchCreate(&branch_, context_, nullptr, nullptr, nullptr,
                            nullptr, nullptr, nullptr, 0, kAdvInterval.count());
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(branch_, nullptr);
  }

  virtual void TearDown() override { ASSERT_EQ(YOGI_DestroyAll(), YOGI_OK); }

  void RunContextInBackground() {
    int res = YOGI_ContextRunInBackground(context_);
    ASSERT_EQ(res, YOGI_OK);
    res = YOGI_ContextWaitForRunning(context_, 1, 0);
    ASSERT_EQ(res, YOGI_OK);
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
  EXPECT_EQ(pt.get("advertising_interval", -1.0f),
            (float)kAdvInterval.count() / 1000.0f);
  EXPECT_GT(pt.get("tcp_server_port", 0), 1024);
  EXPECT_TRUE(std::regex_match(pt.get("start_time", "NOT FOUND"), time_regex));
  EXPECT_EQ(pt.get("active_connections", -1), 0);
}

TEST_F(BranchTest, Advertising) {
  using namespace boost::asio;
  io_context ioc;
  ip::udp::endpoint ep(ip::make_address("0::0"), api::kDefaultAdvPort);
  ip::udp::socket socket(ioc, ep.protocol());
  socket.set_option(ip::udp::socket::reuse_address(true));
  socket.bind(ep);
  socket.set_option(
      ip::multicast::join_group(ip::make_address(api::kDefaultAdvAddress)));

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

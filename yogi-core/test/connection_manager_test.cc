#include "common.h"
#include "../src/api/constants.h"
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>

class ConnectionManagerTest : public Test {
 protected:
  virtual void SetUp() override {
    context_ = CreateContext();
    branch_ = CreateBranch(context_);
  }

  void* context_;
  void* branch_;
};

TEST_F(ConnectionManagerTest, Advertising) {
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

  RunContextInBackground(context_);

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

TEST_F(ConnectionManagerTest, ConnectNormally) {
  // TODO
}

TEST_F(ConnectionManagerTest, ConnectViaTcpServer) {
  // TODO
}

TEST_F(ConnectionManagerTest, ConnectViaTcpClient) {
  // TODO
}

TEST_F(ConnectionManagerTest, Heartbeats) {
  // TODO
}

TEST_F(ConnectionManagerTest, Timeout) {
  // TODO
}

TEST_F(ConnectionManagerTest, Reconnect) {
  // TODO
}

TEST_F(ConnectionManagerTest, DuplicateBranchName) {
  // TODO
}

TEST_F(ConnectionManagerTest, DuplicatePath) {
  // TODO
}

TEST_F(ConnectionManagerTest, InvalidMagicPrefix) {
  // TODO
}

TEST_F(ConnectionManagerTest, IncompatibleVersion) {
  // TODO
}

TEST_F(ConnectionManagerTest, PasswordMismatch) {
  // TODO
}

TEST_F(ConnectionManagerTest, LoopbackConnection) {
  // TODO
}

TEST_F(ConnectionManagerTest, BrokenAdvertisementMessage) {
  // TODO
}

TEST_F(ConnectionManagerTest, BrokenInfoMessage) {
  // TODO
}

TEST_F(ConnectionManagerTest, UnstableConnection) {
  // TODO
}

TEST_F(ConnectionManagerTest, BranchEvents) {
  // TODO
}

TEST_F(ConnectionManagerTest, Connections) {
  CreateBranch(context_, "A");
  // CreateBranch(context_, "B");
  RunContextInBackground(context_);
  getchar();
}

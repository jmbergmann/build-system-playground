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

#include <chrono>

class BroadcastReceiver {
 public:
  BroadcastReceiver(void* branch, int enc = YOGI_ENC_JSON)
      : branch_(branch), data_(16) {
    handler_called_ = false;

    auto res = YOGI_BranchReceiveBroadcastAsync(
        branch_, enc, data_.data(), GetBufferSize(),
        [](int res, int size, void* userarg) {
          auto self = static_cast<BroadcastReceiver*>(userarg);
          self->handler_result_ = res;
          self->data_.resize(static_cast<std::size_t>(size));
          self->handler_called_ = true;
        },
        this);
    EXPECT_EQ(res, YOGI_OK);
  }

  int GetBufferSize() const { return static_cast<int>(data_.size()); }
  int GetHandlerResult() const { return handler_result_; }
  bool BroadcastReceived() const { return handler_called_; }
  std::vector<char> GetReceivedData() const { return data_; }

  template <int N>
  void CheckReceivedDataEquals(const char (&expected_data)[N]) const {
    EXPECT_EQ(data_, std::vector<char>(std::begin(expected_data),
                                       std::end(expected_data)));
  }

  void WaitForBroadcast() const {
    using namespace std::chrono_literals;
    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    while (!BroadcastReceived()) {
      if (clock::now() > start + 1s) {
        throw std::runtime_error("No broadcast received within one second.");
      }
    }
  }

 private:
  void* branch_;
  std::vector<char> data_;
  int handler_result_;
  std::atomic<bool> handler_called_;
};

class BroadcastManagerTest : public TestFixture {
 protected:
  BroadcastManagerTest()
      : context_(CreateContext()),
        branch_a_(CreateBranch(context_, "a")),
        branch_b_(CreateBranch(context_, "b")),
        branch_c_(
            CreateBranch(context_, "c", nullptr, nullptr, nullptr, nullptr, 5)),
        rcv_a_(branch_a_),
        rcv_b_(branch_b_),
        rcv_c_(branch_c_) {
    RunContextUntilBranchesAreConnected(context_,
                                        {branch_a_, branch_b_, branch_c_});
  }

  virtual void TearDown() {
    // To avoid potential seg faults from active receive broadcast operations
    EXPECT_EQ(YOGI_DestroyAll(), YOGI_OK);
  }

  static std::vector<char> MakeBigJsonData(std::size_t size = 10000) {
    std::vector<char> data{'[', '"', '"', ']', '\0'};
    data.insert(data.begin() + 2, size - data.size() + 1, '.');
    return data;
  }

  void* context_;
  void* branch_a_;
  void* branch_b_;
  void* branch_c_;

  BroadcastReceiver rcv_a_;
  BroadcastReceiver rcv_b_;
  BroadcastReceiver rcv_c_;

  const char json_data_[8] = "[1,2,3]";
  const char msgpack_data_[4] = {static_cast<char>(0x93), 0x1, 0x2, 0x3};
};

TEST_F(BroadcastManagerTest, SendJson) {
  RunContextInBackground(context_);

  auto res = YOGI_BranchSendBroadcast(branch_a_, YOGI_ENC_JSON, json_data_,
                                      sizeof(json_data_), YOGI_FALSE);
  ASSERT_EQ(res, YOGI_OK);

  rcv_b_.WaitForBroadcast();
  rcv_b_.CheckReceivedDataEquals(json_data_);
  rcv_c_.WaitForBroadcast();
  EXPECT_FALSE(rcv_a_.BroadcastReceived());
}

TEST_F(BroadcastManagerTest, SendMessagePack) {
  RunContextInBackground(context_);

  auto res =
      YOGI_BranchSendBroadcast(branch_a_, YOGI_ENC_MSGPACK, msgpack_data_,
                               sizeof(msgpack_data_), YOGI_FALSE);
  ASSERT_EQ(res, YOGI_OK);

  rcv_b_.WaitForBroadcast();
  rcv_b_.CheckReceivedDataEquals(json_data_);
  rcv_c_.WaitForBroadcast();
  EXPECT_FALSE(rcv_a_.BroadcastReceived());
}

TEST_F(BroadcastManagerTest, SendBlock) {
  RunContextInBackground(context_);

  auto data = MakeBigJsonData();

  for (int i = 0; i < 10; ++i) {
    int res =
        YOGI_BranchSendBroadcast(branch_c_, YOGI_ENC_JSON, data.data(),
                                 static_cast<int>(data.size()), YOGI_TRUE);
    EXPECT_EQ(res, YOGI_OK);
  }
}

TEST_F(BroadcastManagerTest, SendNoBlock) {
  RunContextInBackground(context_);

  auto data = MakeBigJsonData();
  int res;
  do {
    res = YOGI_BranchSendBroadcast(branch_c_, YOGI_ENC_JSON, data.data(),
                                   static_cast<int>(data.size()), YOGI_FALSE);
  } while (res == YOGI_OK);

  EXPECT_EQ(res, YOGI_ERR_TX_QUEUE_FULL);
}

TEST_F(BroadcastManagerTest, DISABLED_AsyncSendJson) {}

TEST_F(BroadcastManagerTest, DISABLED_AsyncSendMessagePack) {}

TEST_F(BroadcastManagerTest, DISABLED_AsyncSendRetry) {}

TEST_F(BroadcastManagerTest, DISABLED_CancelSend) {}

TEST_F(BroadcastManagerTest, DISABLED_ReceiveJson) {}

TEST_F(BroadcastManagerTest, DISABLED_ReceiveMessagePack) {}

TEST_F(BroadcastManagerTest, DISABLED_CancelReceive) {}

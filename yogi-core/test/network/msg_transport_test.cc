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
#include "../../src/network/msg_transport.h"

#include <random>

class FakeTransport : public network::Transport {
 public:
  FakeTransport(objects::ContextPtr context)
      : network::Transport(context, std::chrono::nanoseconds::max(), true, "") {
  }

  utils::ByteVector tx_data;
  utils::ByteVector rx_data;
  std::size_t tx_send_limit = std::numeric_limits<std::size_t>::max();
  bool dead = false;

 protected:
  virtual void WriteSomeAsync(boost::asio::const_buffer data,
                              TransferSomeHandler handler) override {
    static std::default_random_engine gen;
    std::uniform_int_distribution<std::size_t> dist(1, data.size());
    auto n = std::min(dist(gen), tx_send_limit);
    auto p = static_cast<const char*>(data.data());

    tx_data.insert(tx_data.end(), p, p + n);

    PostHandler(handler, n);
  }

  virtual void ReadSomeAsync(boost::asio::mutable_buffer data,
                             TransferSomeHandler handler) override {
    if (rx_data.empty()) return;

    static std::default_random_engine gen;
    std::uniform_int_distribution<std::size_t> dist(
        1, std::min(data.size(), rx_data.size()));
    auto n = dist(gen);
    std::copy_n(rx_data.begin(), n, static_cast<char*>(data.data()));
    rx_data.erase(rx_data.begin(), rx_data.begin() + n);

    PostHandler(handler, n);
  }

  virtual void Shutdown() override { dead = true; }

 private:
  void PostHandler(TransferSomeHandler handler, std::size_t bytes_transferred) {
    if (dead) {
      GetContext()->Post([=] {
        handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED), bytes_transferred);
      });
    } else {
      GetContext()->Post([=] { handler(api::kSuccess, bytes_transferred); });
    }
  }
};

class MessageTransportTest : public TestFixture {
 protected:
  virtual void SetUp() override {
    context_ = std::make_shared<objects::Context>();
    transport_ = std::make_shared<FakeTransport>(context_);
    uut_ = std::make_shared<network::MessageTransport>(transport_, 8, 8);
  }

  objects::ContextPtr context_;
  std::shared_ptr<FakeTransport> transport_;
  network::MessageTransportPtr uut_;
};

TEST_F(MessageTransportTest, MsgSizeFieldSerialization) {
  using namespace network::internal;

  struct Entry {
    std::size_t ser_length;
    std::size_t value;
  };

  // those are all the corner cases
  const Entry entries[] = {
      // clang-format off
      {1, 0ul},
      {1, 127ul},
      {2, 128ul},
      {2, 16383ul},
      {3, 16384ul},
      {3, 2097151ul},
      {4, 2097152ul},
      {4, 268435455ul},
      {5, 268435456ul},
      {5, 4294967295ul},
      // clang-format on
  };

  for (auto entry : entries) {
    std::array<utils::Byte, 5> buffer = {0};

    auto n = SerializeMsgSizeField(entry.value, &buffer);
    EXPECT_EQ(n, entry.ser_length);

    std::size_t des_value;
    for (std::size_t i = 1; i < entry.ser_length; ++i) {
      EXPECT_FALSE(DeserializeMsgSizeField(buffer, i, &des_value));
    }

    EXPECT_TRUE(DeserializeMsgSizeField(buffer, entry.ser_length, &des_value));
    EXPECT_EQ(des_value, entry.value);
  }
}

TEST_F(MessageTransportTest, TrySend) {
  uut_->Start();

  utils::ByteVector data{1, 2, 3, 4, 5};
  EXPECT_TRUE(uut_->TrySend(data));
  EXPECT_FALSE(uut_->TrySend(data));
  context_->Poll();
  EXPECT_TRUE(uut_->TrySend(data));
  context_->Poll();

  EXPECT_EQ(transport_->tx_data,
            (utils::ByteVector{5, 1, 2, 3, 4, 5, 5, 1, 2, 3, 4, 5}));
}

TEST_F(MessageTransportTest, TrySendTransportFailure) {
  uut_->Start();

  transport_->Close();
  utils::ByteVector data{1, 2, 3, 4, 5};
  uut_->TrySend(data);
  context_->Poll();

  EXPECT_THROW_ERROR(uut_->TrySend(data), YOGI_ERR_RW_SOCKET_FAILED);
}

TEST_F(MessageTransportTest, SendAsync) {
  transport_->tx_send_limit = 1;
  uut_->Start();

  utils::ByteVector data{1, 2, 3, 4, 5, 6};
  bool called = false;
  uut_->SendAsync(data, [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    called = true;
  });
  context_->PollOne();
  context_->PollOne();
  EXPECT_TRUE(called);

  called = false;
  uut_->SendAsync(data, [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    called = true;
  });
  context_->PollOne();
  context_->PollOne();
  EXPECT_FALSE(called);
  context_->Poll();
  EXPECT_TRUE(called);

  EXPECT_EQ(transport_->tx_data,
            (utils::ByteVector{6, 1, 2, 3, 4, 5, 6, 6, 1, 2, 3, 4, 5, 6}));
}

TEST_F(MessageTransportTest, AsyncSendTransportFailure) {
  uut_->Start();

  transport_->Close();
  utils::ByteVector data{1, 2, 3, 4, 5, 6};
  uut_->TrySend(data);

  bool called = false;
  uut_->SendAsync(data, [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
    called = true;
  });
  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, CancelSend) {
  uut_->Start();

  utils::ByteVector data{1, 2, 3, 4, 5, 6};
  EXPECT_TRUE(uut_->TrySend(data));

  bool called = false;
  uut_->SendAsync(data, [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
    called = true;
  });
  uut_->CancelSend();

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, ReceiveAsync) {
  transport_->rx_data = utils::ByteVector{5, 1, 2, 3, 4, 5, 4, 1, 2, 3, 4};
  uut_->Start();
  context_->Poll();

  utils::ByteVector data(5);
  bool called = false;
  uut_->ReceiveAsync(&data, [&](auto& res, auto size) {
    EXPECT_EQ(res, api::kSuccess);
    EXPECT_EQ(size, 5);
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
  EXPECT_EQ(data, (utils::ByteVector{1, 2, 3, 4, 5}));

  data = utils::ByteVector(4);
  called = false;
  uut_->ReceiveAsync(&data, [&](auto& res, auto size) {
    EXPECT_EQ(res, api::kSuccess);
    EXPECT_EQ(size, 4);
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
  EXPECT_EQ(data, (utils::ByteVector{1, 2, 3, 4}));
}

TEST_F(MessageTransportTest, ReceiveAsyncTransportFailure) {
  transport_->rx_data = utils::ByteVector{5, 1, 2, 3, 4, 5, 4, 1, 2, 3, 4};
  uut_->Start();
  transport_->Close();

  utils::ByteVector data(5);
  bool called = false;
  uut_->ReceiveAsync(&data, [&](auto& res, auto) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, CancelReceive) {
  uut_->Start();

  utils::ByteVector data(10);
  bool called = false;
  uut_->ReceiveAsync(&data, [&](auto& res, auto) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
    called = true;
  });
  uut_->CancelReceive();

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, Close) {
  uut_->Start();
  EXPECT_FALSE(transport_->dead);
  uut_->Close();
  EXPECT_TRUE(transport_->dead);
}

TEST_F(MessageTransportTest, DISABLED_Stress) {}

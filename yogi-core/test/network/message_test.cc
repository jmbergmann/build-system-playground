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
#include "../../src/network/messages.h"
using namespace network;

#include <nlohmann/json.hpp>

class FakeMessage : public Message {
 public:
  using network::Message::Message;

  virtual std::string ToString() const override { return {}; }
};

TEST(MessagesTest, Constructors) {
  auto msg1 = FakeMessage(MessageType::kHeartbeat);
  EXPECT_TRUE(msg1.GetMessageAsBytes().empty());

  auto msg2 = FakeMessage(MessageType::kAcknowledge);
  EXPECT_GT(msg2.GetMessageAsBytes().size(), 0);

  auto type = MessageType::kBroadcast;
  auto hdr = boost::asio::buffer("Header");
  auto usr_msgpack = boost::asio::buffer("a");
  auto usr_json = boost::asio::buffer("{\"x\": 456}");
  const unsigned char usr_invalid_data[] = {200};
  auto usr_invalid = boost::asio::buffer(usr_invalid_data);

  EXPECT_NO_THROW(FakeMessage(type, hdr, usr_msgpack, api::Encoding::kMsgPack));
  EXPECT_NO_THROW(FakeMessage(type, hdr, usr_json, api::Encoding::kJson));
  EXPECT_NO_THROW(FakeMessage(type, usr_json, api::Encoding::kJson));
  EXPECT_NO_THROW(FakeMessage(type, usr_msgpack, api::Encoding::kMsgPack));

  EXPECT_THROW_ERROR(FakeMessage(type, hdr, usr_invalid, api::Encoding::kJson),
                     YOGI_ERR_PARSING_JSON_FAILED);
  EXPECT_THROW_ERROR(
      FakeMessage(type, hdr, usr_invalid, api::Encoding::kMsgPack),
      YOGI_ERR_INVALID_USER_MSGPACK);
  EXPECT_THROW_ERROR(FakeMessage(type, usr_invalid, api::Encoding::kJson),
                     YOGI_ERR_PARSING_JSON_FAILED);
  EXPECT_THROW_ERROR(FakeMessage(type, usr_invalid, api::Encoding::kMsgPack),
                     YOGI_ERR_INVALID_USER_MSGPACK);
}

TEST(MessagesTest, GetType) {
  auto msg = FakeMessage(MessageType::kBroadcast);
  EXPECT_EQ(MessageType::kBroadcast, msg.GetType());
}

TEST(MessagesTest, GetSize) {
  auto msg = FakeMessage(MessageType::kBroadcast, boost::asio::buffer("Header"),
                         boost::asio::buffer("a"), api::Encoding::kMsgPack);
  EXPECT_EQ(msg.GetSize(), 1 + 7 + 2);
}

TEST(MessagesTest, GetMessageAsBytes) {
  auto msg = FakeMessage(MessageType::kBroadcast, boost::asio::buffer("X"),
                         boost::asio::buffer("a"), api::Encoding::kMsgPack);
  auto bytes = msg.GetMessageAsBytes();
  EXPECT_EQ(bytes, (utils::ByteVector{msg.GetType(), 'X', '\0', 'a', '\0'}));

  msg.GetMessageAsSharedBytes();
  EXPECT_EQ(msg.GetMessageAsBytes(), bytes);
}

TEST(MessagesTest, GetMessageAsSharedBytes) {
  auto msg = FakeMessage(MessageType::kBroadcast, boost::asio::buffer("X"),
                         boost::asio::buffer("a"), api::Encoding::kMsgPack);
  auto bytes = msg.GetMessageAsSharedBytes();
  EXPECT_EQ(*bytes, (utils::ByteVector{msg.GetType(), 'X', '\0', 'a', '\0'}));
}

TEST(MessagesTest, JsonToMsgPackConversion) {
  auto msg =
      FakeMessage(MessageType::kBroadcast, boost::asio::buffer("{\"x\": 456}"),
                  api::Encoding::kJson);

  auto data = msg.GetMessageAsBytes();
  data.erase(data.begin(), data.begin() + 1);

  auto json = nlohmann::json::from_msgpack(data);
  EXPECT_EQ(json.value("x", -1), 456);
}

TEST(MessagesTest, CreateFromBytes) {
  utils::ByteVector bytes = {MessageType::kBroadcast, 0x93, 0x01, 0x02, 0x03};

  bool called = false;
  FakeMessage::CreateFromBytes(bytes, [&](const Message& msg) {
    EXPECT_EQ(msg.GetType(), MessageType::kBroadcast);
    called = true;
  });

  EXPECT_TRUE(called);
}

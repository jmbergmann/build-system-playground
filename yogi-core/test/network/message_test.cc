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
#include "../../src/network/message.h"

#include <nlohmann/json.hpp>

using namespace network;

TEST(MessageTest, Constructors) {
  auto msg1 = Message(MessageType::kHeartbeat);
  EXPECT_TRUE(msg1.GetMessageAsBytes().empty());

  auto msg2 = Message(MessageType::kAcknowledge);
  EXPECT_GT(msg2.GetMessageAsBytes().size(), 0);

  auto type = MessageType::kBroadcast;
  auto hdr = boost::asio::buffer("Header");
  auto usr_msgpack = boost::asio::buffer("a");
  auto usr_json = boost::asio::buffer("{\"x\": 456}");
  const unsigned char usr_invalid_data[] = {200};
  auto usr_invalid = boost::asio::buffer(usr_invalid_data);

  EXPECT_NO_THROW(Message(type, hdr, usr_msgpack, api::Encoding::kMsgPack));
  EXPECT_NO_THROW(Message(type, hdr, usr_json, api::Encoding::kJson));
  EXPECT_NO_THROW(Message(type, usr_json, api::Encoding::kJson));
  EXPECT_NO_THROW(Message(type, usr_msgpack, api::Encoding::kMsgPack));

  EXPECT_THROW_ERROR(Message(type, hdr, usr_invalid, api::Encoding::kJson),
                     YOGI_ERR_PARSING_JSON_FAILED);
  EXPECT_THROW_ERROR(Message(type, hdr, usr_invalid, api::Encoding::kMsgPack),
                     YOGI_ERR_INVALID_USER_MSGPACK);
  EXPECT_THROW_ERROR(Message(type, usr_invalid, api::Encoding::kJson),
                     YOGI_ERR_PARSING_JSON_FAILED);
  EXPECT_THROW_ERROR(Message(type, usr_invalid, api::Encoding::kMsgPack),
                     YOGI_ERR_INVALID_USER_MSGPACK);
}

TEST(MessageTest, GetType) {
  auto msg = Message(MessageType::kBroadcast);
  EXPECT_EQ(MessageType::kBroadcast, msg.GetType());
}

TEST(MessageTest, GetSize) {
  auto msg = Message(MessageType::kBroadcast, boost::asio::buffer("Header"),
                     boost::asio::buffer("a"), api::Encoding::kMsgPack);
  EXPECT_EQ(msg.GetSize(), 1 + 7 + 2);
}

TEST(MessageTest, GetHeader) {
  auto msg = Message(MessageType::kBroadcast, boost::asio::buffer("AB"));
  EXPECT_EQ(msg.GetHeader(), (utils::ByteVector{'A', 'B', '\0'}));
}

TEST(MessageTest, GetUserDataSize) {
  auto msg = Message(MessageType::kBroadcast, boost::asio::buffer("a"),
                     api::Encoding::kMsgPack);
  EXPECT_EQ(msg.GetUserDataSize(), 2);
}

TEST(MessageTest, GetMessageAsBytes) {
  auto msg = Message(MessageType::kBroadcast, boost::asio::buffer("X"),
                     boost::asio::buffer("a"), api::Encoding::kMsgPack);
  auto bytes = msg.GetMessageAsBytes();
  EXPECT_EQ(bytes, (utils::ByteVector{msg.GetType(), 'X', '\0', 'a', '\0'}));

  msg.GetMessageAsSharedBytes();
  EXPECT_EQ(msg.GetMessageAsBytes(), bytes);
}

TEST(MessageTest, GetMessageAsSharedBytes) {
  auto msg = Message(MessageType::kBroadcast, boost::asio::buffer("X"),
                     boost::asio::buffer("a"), api::Encoding::kMsgPack);
  auto bytes = msg.GetMessageAsSharedBytes();
  EXPECT_EQ(*bytes, (utils::ByteVector{msg.GetType(), 'X', '\0', 'a', '\0'}));
}

TEST(MessageTest, JsonToMsgPackConversion) {
  auto msg = Message(MessageType::kBroadcast,
                     boost::asio::buffer("{\"x\": 456}"), api::Encoding::kJson);

  auto data = msg.GetMessageAsBytes();
  data.erase(data.begin(), data.end() - msg.GetUserDataSize());

  auto json = nlohmann::json::from_msgpack(data);
  EXPECT_EQ(json.value("x", -1), 456);
}

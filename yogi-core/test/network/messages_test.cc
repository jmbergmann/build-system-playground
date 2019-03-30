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

class FakeOutgoingMessage : public OutgoingMessage,
                            public MessageT<MessageType::kBroadcast> {
 public:
  using MessageT::MakeMsgBytes;

  FakeOutgoingMessage(utils::SmallByteVector serialized_msg)
      : OutgoingMessage(serialized_msg) {}

  virtual std::string ToString() const override { return {}; }
};

TEST(MessagesTest, UserDataJson) {
  auto user_data =
      UserData(boost::asio::buffer("{\"x\": 456}"), api::Encoding::kJson);

  utils::SmallByteVector buffer;
  EXPECT_NO_THROW(user_data.SerializeTo(&buffer));

  auto json = nlohmann::json::from_msgpack(buffer);
  EXPECT_EQ(json.value("x", -1), 456);

  const utils::Byte invalid_data[] = {200};
  user_data = UserData(boost::asio::buffer(invalid_data), api::Encoding::kJson);
  EXPECT_THROW_ERROR(user_data.SerializeTo(&buffer),
                     YOGI_ERR_PARSING_JSON_FAILED);
}

TEST(MessageTest, UserDataMsgPack) {
  auto data = utils::SmallByteVector{0x93, 0x1, 0x2, 0x3};
  auto user_data = UserData(boost::asio::buffer(data.data(), data.size()),
                            api::Encoding::kMsgPack);

  utils::SmallByteVector buffer;
  EXPECT_NO_THROW(user_data.SerializeTo(&buffer));
  EXPECT_EQ(buffer, data);

  const utils::Byte invalid_data[] = {200};
  user_data =
      UserData(boost::asio::buffer(invalid_data), api::Encoding::kMsgPack);
  EXPECT_THROW_ERROR(user_data.SerializeTo(&buffer),
                     YOGI_ERR_INVALID_USER_MSGPACK);
}

TEST(MessagesTest, GetType) {
  auto fakeType = FakeOutgoingMessage::kMessageType;
  EXPECT_EQ(fakeType, MessageType::kBroadcast);
  EXPECT_EQ(fakeType, FakeOutgoingMessage({}).GetType());
}

TEST(MessagesTest, MakeMsgBytes) {
  auto type = FakeOutgoingMessage::kMessageType;
  auto user_data =
      UserData(boost::asio::buffer("x", 1), api::Encoding::kMsgPack);

  auto bytes = FakeOutgoingMessage::MakeMsgBytes();
  EXPECT_EQ(bytes, utils::SmallByteVector{type});

  bytes = FakeOutgoingMessage::MakeMsgBytes(user_data);
  EXPECT_EQ(bytes, (utils::SmallByteVector{type, 'x'}));

  bytes = FakeOutgoingMessage::MakeMsgBytes(std::make_tuple(true, false, 123));
  EXPECT_EQ(bytes, (utils::SmallByteVector{type, 0x93, 0xc3, 0xc2, 0x7b}));

  bytes = FakeOutgoingMessage::MakeMsgBytes(std::make_tuple(true, false, 123),
                                            user_data);
  EXPECT_EQ(bytes, (utils::SmallByteVector{type, 0x93, 0xc3, 0xc2, 0x7b, 'x'}));
}

TEST(MessagesTest, GetSize) {
  auto msg = FakeOutgoingMessage({1, 2, 3});
  EXPECT_EQ(msg.GetSize(), 3);
}

TEST(MessagesTest, Serialize) {
  auto msg = FakeOutgoingMessage({1, 2, 3});
  auto bytes = msg.Serialize();
  EXPECT_EQ(bytes, (utils::SmallByteVector{1, 2, 3}));

  msg.SerializeShared();
  EXPECT_EQ(msg.Serialize(), bytes);
}

TEST(MessagesTest, SerializeShared) {
  auto msg = FakeOutgoingMessage({1, 2, 3});
  auto bytes = msg.SerializeShared();
  EXPECT_EQ(*bytes, (utils::SmallByteVector{1, 2, 3}));
}

TEST(MessagesTest, Deserialize) {
  utils::ByteVector bytes = {MessageType::kBroadcast, 0x93, 0x01, 0x02, 0x03};

  bool called = false;
  IncomingMessage::Deserialize(bytes, [&](const IncomingMessage& msg) {
    EXPECT_EQ(msg.GetType(), MessageType::kBroadcast);

    auto bcm = dynamic_cast<const messages::BroadcastIncoming*>(&msg);
    ASSERT_NE(bcm, nullptr);

    utils::SmallByteVector user_data;
    bcm->GetUserData().SerializeTo(&user_data);
    EXPECT_EQ(user_data,
              utils::SmallByteVector(bytes.begin() + 1, bytes.end()));

    called = true;
  });

  EXPECT_TRUE(called);
}

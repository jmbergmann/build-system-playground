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

#include <yogi/payload.h>

class PayloadTest : public testing::Test {
 protected:
  PayloadTest()
      : data_("Hello"), payload_(data_, 123, yogi::EncodingType::kMsgpack) {}

  const void* data_;
  const yogi::Payload payload_;
};

TEST_F(PayloadTest, Getters) {
  EXPECT_EQ(payload_.Data(), data_);
  EXPECT_EQ(payload_.Size(), 123);
  EXPECT_EQ(payload_.Encoding(), yogi::EncodingType::kMsgpack);
}

TEST_F(PayloadTest, Copy) {
  auto pl = yogi::Payload("Blabla", 555, yogi::EncodingType::kJson);
  pl = payload_;
  EXPECT_EQ(pl.Data(), payload_.Data());
  EXPECT_EQ(pl.Size(), payload_.Size());
  EXPECT_EQ(pl.Encoding(), payload_.Encoding());
}

TEST_F(PayloadTest, ConstructFromJsonView) {
  yogi::Json json = {12345};
  auto view = yogi::JsonView(json);
  auto pl = yogi::Payload(json);

  auto s = json.dump();
  EXPECT_EQ(s, static_cast<const char*>(pl.Data()));
  EXPECT_EQ(pl.Encoding(), yogi::EncodingType::kJson);
}

TEST_F(PayloadTest, ConstructFromMsgpackView) {
  std::string msgpack = "Hello";
  auto view = yogi::MsgpackView(msgpack);
  auto pl = yogi::Payload(view);

  EXPECT_EQ(msgpack, std::string(static_cast<const char*>(pl.Data()),
                                 static_cast<std::size_t>(pl.Size())));
  EXPECT_EQ(pl.Encoding(), yogi::EncodingType::kMsgpack);
}
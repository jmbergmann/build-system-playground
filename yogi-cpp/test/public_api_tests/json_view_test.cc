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

TEST(JsonViewTest, Default) {
  auto view = yogi::JsonView();
  EXPECT_EQ(view.Data(), nullptr);
  EXPECT_EQ(view.Size(), 0);
}

TEST(JsonViewTest, ConstCharString) {
  const char* s = "Hello";
  auto view = yogi::JsonView(s);
  EXPECT_EQ(view.Data(), s);
  EXPECT_EQ(view.Size(), strlen(s));
}

TEST(JsonViewTest, StdString) {
  std::string s = "Hello";
  auto view = yogi::JsonView(s);
  EXPECT_EQ(std::string(view.Data()), s);
  EXPECT_EQ(view.Size(), s.size());
}

TEST(JsonViewTest, JsonObject) {
  yogi::Json json = {12345};
  auto view = yogi::JsonView(json);
  EXPECT_EQ(json.dump(), view.Data());
  EXPECT_EQ(view.Size(), json.dump().size());
}

TEST(JsonViewTest, ConversionOperator) {
  const char* s = "Hello";
  auto view = yogi::JsonView(s);
  EXPECT_EQ(static_cast<const char*>(view), view.Data());

  yogi::Json json = {12345};
  auto view2 = yogi::JsonView(json);
  EXPECT_EQ(static_cast<const char*>(view), view.Data());
}

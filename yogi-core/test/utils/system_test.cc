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

#include <algorithm>
#include <regex>

TEST(SystemTest, GetHostname) { EXPECT_FALSE(utils::GetHostname().empty()); }

TEST(SystemTest, GetProcessId) { EXPECT_GT(utils::GetProcessId(), 0); }

TEST(SystemTest, GetCurrentThreadId) {
  EXPECT_GT(utils::GetCurrentThreadId(), 0);
}

TEST(SystemTest, GetNetworkInterfaces) {
  auto ifs = utils::GetNetworkInterfaces();
  ASSERT_FALSE(ifs.empty());

  auto localhost_found = false;

  for (auto& info : ifs) {
    EXPECT_FALSE(info.name.empty());
    EXPECT_FALSE(info.identifier.empty());
    EXPECT_FALSE(info.mac.empty() && info.addresses.empty());

    if (!info.mac.empty()) {
      std::regex re("^([0-9a-f][0-9a-f]:){5}[0-9a-f][0-9a-f]$");
      std::smatch m;
      EXPECT_TRUE(std::regex_match(info.mac, m, re)) << "String: " << info.mac;
    }

    bool is_loopback =
        std::count_if(info.addresses.begin(), info.addresses.end(),
                      [](auto& addr) { return addr.is_loopback(); }) > 0;
    EXPECT_EQ(is_loopback, info.is_loopback);
    localhost_found |= is_loopback;
  }

  EXPECT_TRUE(localhost_found);
}

TEST(SystemTest, DISABLED_GetFilteredNetworkInterfaces) {}

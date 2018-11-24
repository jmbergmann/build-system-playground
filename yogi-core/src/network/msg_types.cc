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

#include "msg_types.h"

#define YOGI_MSGTYPE_CASE(enum_element) \
  case MessageType::enum_element:  \
    os << #enum_element;

std::ostream& operator<<(std::ostream& os,
                         const network::MessageType& msg_type) {
  using network::MessageType;

  switch (msg_type) {
    YOGI_MSGTYPE_CASE(kHeartbeat);
    YOGI_MSGTYPE_CASE(kAcknowledge);
    YOGI_MSGTYPE_CASE(kBroadcast);

    default:
      os << "INVALID MESSAGE TYPE";
  }

  return os;
}

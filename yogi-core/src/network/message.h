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

#pragma once

#include "../config.h"
#include "../api/enums.h"
#include "../utils/types.h"
#include "msg_types.h"

#include <boost/asio/buffer.hpp>
#include <fstream>
#include <array>

namespace network {

class Message {
 public:
  explicit Message(MessageType type);
  Message(MessageType type, boost::asio::const_buffer header,
          boost::asio::const_buffer user_data, api::Encoding user_enc);
  Message(MessageType type, boost::asio::const_buffer header);
  Message(MessageType type, boost::asio::const_buffer user_data,
          api::Encoding user_enc);

  MessageType GetType() const { return type_; }
  std::size_t GetSize() const;
  utils::ByteVector GetHeader() const;
  std::size_t GetUserDataSize() const;
  const utils::ByteVector& GetMessageAsBytes() const;
  utils::SharedByteVector GetMessageAsSharedBytes();

 private:
  void PopulateMsg(boost::asio::const_buffer header,
                   boost::asio::const_buffer user_data, api::Encoding user_enc);
  utils::ByteVector CheckAndConvertUserDataFromJsonToMsgPack(
      boost::asio::const_buffer user_data);
  void CheckUserDataIsValidMsgPack(boost::asio::const_buffer user_data);

  const MessageType type_;
  const std::size_t header_size_;
  utils::ByteVector msg_;
  utils::SharedByteVector shared_msg_;
};

}  // namespace network

std::ostream& operator<<(std::ostream& os, const network::Message& msg);

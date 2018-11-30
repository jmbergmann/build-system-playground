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

#include <boost/asio/buffer.hpp>
#include <fstream>
#include <array>

namespace network {

enum MessageType : utils::Byte {
  kHeartbeat,
  kAcknowledge,
  kBroadcast,
};

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
  const utils::ByteVector& GetMessageAsBytes() const;
  utils::SharedByteVector GetMessageAsSharedBytes();

  virtual std::string ToString() const =0;

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

namespace messages {

// This is a message whose length is zero (msg type is omitted)
class Heartbeat : public Message {
 public:
  Heartbeat();
  virtual std::string ToString() const override;
};

class Acknowledge : public Message {
 public:
  Acknowledge();
  virtual std::string ToString() const override;
};

class Broadcast : public Message {
 public:
  Broadcast(boost::asio::const_buffer data, api::Encoding enc);
  virtual std::string ToString() const override;
};

}  // namespace messages
}  // namespace network

std::ostream& operator<<(std::ostream& os, const network::Message& msg);

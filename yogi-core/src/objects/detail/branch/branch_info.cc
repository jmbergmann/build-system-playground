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

#include "branch_info.h"
#include "../../../api/constants.h"
#include "../../../utils/ip.h"
#include "../../../utils/serialize.h"
#include "../../../utils/system.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace objects {
namespace detail {
namespace {

template <typename Field>
void DeserializeField(Field* field, const utils::ByteVector& msg,
                      utils::ByteVector::const_iterator* it) {
  if (!utils::Deserialize<Field>(field, msg, it)) {
    throw api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED);
  }
}

}  // anonymous namespace

BranchInfoPtr BranchInfo::CreateLocal(
    std::string name, std::string description, std::string net_name,
    std::string path, const boost::asio::ip::tcp::endpoint& tcp_ep,
    const std::chrono::nanoseconds& timeout,
    const std::chrono::nanoseconds& adv_interval, bool ghost_mode) {
  auto info = std::make_shared<BranchInfo>();
  info->uuid_ = boost::uuids::random_generator()();
  info->name_ = name;
  info->description_ = description;
  info->net_name_ = net_name;
  info->path_ = path;
  info->hostname_ = utils::GetHostname();
  info->pid_ = utils::GetProcessId();
  info->tcp_ep_ = tcp_ep;
  info->start_time_ = utils::Timestamp::Now();
  info->timeout_ = timeout;
  info->adv_interval_ = adv_interval;
  info->ghost_mode_ = ghost_mode;

  info->PopulateMessages();
  info->PopulateJson();

  return info;
}

BranchInfoPtr BranchInfo::CreateFromInfoMessage(
    const utils::ByteVector& info_msg, const boost::asio::ip::address& addr) {
  auto info = std::make_shared<BranchInfo>();

  unsigned short port;
  auto res = DeserializeAdvertisingMessage(&info->uuid_, &port, info_msg);
  if (res.IsError()) {
    throw res.ToError();
  }

  info->tcp_ep_.port(port);
  info->tcp_ep_.address(addr);

  auto it = info_msg.cbegin() + kInfoMessageHeaderSize;
  DeserializeField(&info->name_, info_msg, &it);
  DeserializeField(&info->description_, info_msg, &it);
  DeserializeField(&info->net_name_, info_msg, &it);
  DeserializeField(&info->path_, info_msg, &it);
  DeserializeField(&info->hostname_, info_msg, &it);
  DeserializeField(&info->pid_, info_msg, &it);
  DeserializeField(&info->start_time_, info_msg, &it);
  DeserializeField(&info->timeout_, info_msg, &it);
  DeserializeField(&info->adv_interval_, info_msg, &it);
  DeserializeField(&info->ghost_mode_, info_msg, &it);

  info->PopulateJson();

  return info;
}

api::Result BranchInfo::DeserializeAdvertisingMessage(
    boost::uuids::uuid* uuid, unsigned short* tcp_port,
    const utils::ByteVector& adv_msg) {
  YOGI_ASSERT(adv_msg.size() >= kAdvertisingMessageSize);

  auto res = CheckMagicPrefixAndVersion(adv_msg);
  if (res.IsError()) {
    return res;
  }

  auto it = adv_msg.cbegin() + 7;
  utils::Deserialize(uuid, adv_msg, &it);
  utils::Deserialize(tcp_port, adv_msg, &it);

  return api::kSuccess;
}

api::Result BranchInfo::DeserializeInfoMessageBodySize(
    std::size_t* body_size, const utils::ByteVector& info_msg_hdr) {
  YOGI_ASSERT(info_msg_hdr.size() >= kInfoMessageHeaderSize);

  auto res = CheckMagicPrefixAndVersion(info_msg_hdr);
  if (res.IsError()) {
    return res;
  }

  auto it = info_msg_hdr.cbegin() + kAdvertisingMessageSize;
  utils::Deserialize(body_size, info_msg_hdr, &it);

  return api::kSuccess;
}

api::Result BranchInfo::CheckMagicPrefixAndVersion(
    const utils::ByteVector& adv_msg) {
  YOGI_ASSERT(adv_msg.size() >= kAdvertisingMessageSize);
  if (std::memcmp(adv_msg.data(), "YOGI", 5)) {
    return api::Error(YOGI_ERR_INVALID_MAGIC_PREFIX);
  }

  if (adv_msg[5] != api::kVersionMajor || adv_msg[6] != api::kVersionMinor) {
    return api::Error(YOGI_ERR_INCOMPATIBLE_VERSION);
  }

  return api::kSuccess;
}

void BranchInfo::PopulateMessages() {
  utils::ByteVector buffer{'Y', 'O', 'G', 'I', 0};
  buffer.push_back(api::kVersionMajor);
  buffer.push_back(api::kVersionMinor);
  utils::Serialize(&buffer, uuid_);
  utils::Serialize(&buffer, tcp_ep_.port());

  YOGI_ASSERT(buffer.size() == kAdvertisingMessageSize);
  adv_msg_ = utils::MakeSharedByteVector(buffer);

  info_msg_ = utils::MakeSharedByteVector(buffer);

  buffer.clear();
  utils::Serialize(&buffer, name_);
  utils::Serialize(&buffer, description_);
  utils::Serialize(&buffer, net_name_);
  utils::Serialize(&buffer, path_);
  utils::Serialize(&buffer, hostname_);
  utils::Serialize(&buffer, pid_);
  utils::Serialize(&buffer, start_time_);
  utils::Serialize(&buffer, timeout_);
  utils::Serialize(&buffer, adv_interval_);
  utils::Serialize(&buffer, ghost_mode_);

  utils::Serialize(&*info_msg_, buffer.size());
  YOGI_ASSERT(info_msg_->size() == kInfoMessageHeaderSize);
  info_msg_->insert(info_msg_->end(), buffer.begin(), buffer.end());
}

void BranchInfo::PopulateJson() {
  float timeout = -1;
  if (timeout_ != timeout_.max()) {
    timeout = static_cast<float>(timeout_.count()) / 1e9f;
  }

  float adv_interval = -1;
  if (adv_interval_ != adv_interval_.max()) {
    adv_interval = static_cast<float>(adv_interval_.count()) / 1e9f;
  }

  json_ = {
      {"uuid", boost::uuids::to_string(uuid_)},
      {"name", name_},
      {"description", description_},
      {"network_name", net_name_},
      {"path", path_},
      {"hostname", hostname_},
      {"pid", pid_},
      {"tcp_server_address", utils::MakeIpAddressString(tcp_ep_)},
      {"tcp_server_port", tcp_ep_.port()},
      {"start_time", start_time_.ToJavaScriptString()},
      {"timeout", timeout},
      {"advertising_interval", adv_interval},
      {"ghost_mode", ghost_mode_},
  };
}

}  // namespace detail
}  // namespace objects

std::ostream& operator<<(std::ostream& os,
                         const objects::detail::BranchInfoPtr& info) {
  if (info) {
    os << '[' << info->GetUuid() << ']';
  } else {
    os << "[????????-????-????-????-????????????]";
  }

  return os;
}

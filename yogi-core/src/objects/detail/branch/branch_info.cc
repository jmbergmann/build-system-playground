#include "branch_info.h"
#include "../../../api/constants.h"
#include "../../../utils/serialize.h"
#include "../../../utils/system.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace objects {
namespace detail {
namespace {

template <typename Field>
void DeserializeField(Field* field, const std::vector<char>& msg,
                      std::vector<char>::const_iterator* it) {
  if (!utils::Deserialize<Field>(field, msg, it)) {
    throw api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED);
  }
}

}  // anonymous namespace

BranchInfoPtr BranchInfo::CreateLocal(
    std::string name, std::string description, std::string net_name,
    std::string path, const boost::asio::ip::tcp::endpoint& tcp_ep,
    const std::chrono::nanoseconds& timeout,
    const std::chrono::nanoseconds& adv_interval) {
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

  info->PopulateMessages();
  info->PopulateJson();

  return info;
}

BranchInfoPtr BranchInfo::CreateFromInfoMessage(
    const std::vector<char> info_msg, const boost::asio::ip::address& addr) {
  auto info = std::make_shared<BranchInfo>();

  unsigned short port;
  if (auto err = DeserializeAdvertisingMessage(&info->uuid_, &port, info_msg)) {
    throw err;
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

  info->PopulateJson();

  return info;
}

api::Error BranchInfo::DeserializeAdvertisingMessage(
    boost::uuids::uuid* uuid, unsigned short* tcp_port,
    const std::vector<char>& adv_msg) {
  YOGI_ASSERT(adv_msg.size() >= kAdvertisingMessageSize);
  if (auto err = CheckMagicPrefixAndVersion(adv_msg)) {
    return err;
  }

  auto it = adv_msg.cbegin() + 7;
  utils::Deserialize(uuid, adv_msg, &it);
  utils::Deserialize(tcp_port, adv_msg, &it);

  return api::kSuccess;
}

api::Error BranchInfo::DeserializeInfoMessageBodySize(
    std::size_t* body_size, const std::vector<char>& info_msg_hdr) {
  YOGI_ASSERT(info_msg_hdr.size() >= kInfoMessageHeaderSize);
  if (auto err = CheckMagicPrefixAndVersion(info_msg_hdr)) {
    return err;
  }

  auto it = info_msg_hdr.cbegin() + kAdvertisingMessageSize;
  utils::Deserialize(body_size, info_msg_hdr, &it);

  return api::kSuccess;
}

api::Error BranchInfo::CheckMagicPrefixAndVersion(
    const std::vector<char>& adv_msg) {
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
  adv_msg_ = {'Y', 'O', 'G', 'I', 0};
  adv_msg_.push_back(api::kVersionMajor);
  adv_msg_.push_back(api::kVersionMinor);
  utils::Serialize(&adv_msg_, uuid_);
  utils::Serialize(&adv_msg_, tcp_ep_.port());
  YOGI_ASSERT(adv_msg_.size() == kAdvertisingMessageSize);

  std::vector<char> buffer;
  utils::Serialize(&buffer, name_);
  utils::Serialize(&buffer, description_);
  utils::Serialize(&buffer, net_name_);
  utils::Serialize(&buffer, path_);
  utils::Serialize(&buffer, hostname_);
  utils::Serialize(&buffer, pid_);
  utils::Serialize(&buffer, start_time_);
  utils::Serialize(&buffer, timeout_);
  utils::Serialize(&buffer, adv_interval_);

  info_msg_ = adv_msg_;
  utils::Serialize(&info_msg_, buffer.size());
  YOGI_ASSERT(info_msg_.size() == kInfoMessageHeaderSize);
  info_msg_.insert(info_msg_.end(), buffer.begin(), buffer.end());
}

void BranchInfo::PopulateJson() {
  json_ = {
      {"uuid", boost::uuids::to_string(uuid_)},
      {"name", name_},
      {"description", description_},
      {"net_name", net_name_},
      {"path", path_},
      {"hostname", hostname_},
      {"pid", pid_},
      {"tcp_server_address", tcp_ep_.address().to_string()},
      {"tcp_server_port", tcp_ep_.port()},
      {"start_time", start_time_.ToJavaScriptString()},
      {"timeout", static_cast<float>(timeout_.count()) / 1000'000'000.0f},
      {"advertising_interval",
       static_cast<float>(adv_interval_.count()) / 1000'000'000.0f},
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

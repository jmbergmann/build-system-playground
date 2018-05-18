#include "branch_info.h"
#include "../../../api/constants.h"
#include "../../../utils/serialize.h"
#include "../../../utils/system.h"

#include <boost/uuid/uuid_io.hpp>

namespace objects {
namespace detail {

std::size_t BranchInfo::GetAdvertisingMessageSize() {
  static auto size = LocalBranchInfo().MakeAdvertisingMessage().size();
  return size;
}

std::size_t BranchInfo::GetInfoMessageHeaderSize() {
  return GetAdvertisingMessageSize() + 4;
}

api::Error BranchInfo::CheckAdvertisingMessageValidity(
    const std::vector<char>& msg) {
  if (std::memcmp(msg.data(), "YOGI", 5) != 0) {
    return api::Error(YOGI_ERR_INVALID_MAGIC_PREFIX);
  }

  if (msg[5] != api::kVersionMajor || msg[6] != api::kVersionMinor) {
    return api::Error(YOGI_ERR_INCOMPATIBLE_VERSION);
  }

  return api::kSuccess;
}

nlohmann::json BranchInfo::ToJson() const {
  return {
      {"uuid", boost::uuids::to_string(uuid)},
      {"name", name},
      {"description", description},
      {"net_name", net_name},
      {"path", path},
      {"hostname", hostname},
      {"pid", pid},
      {"tcp_server_address", tcp_ep.address().to_string()},
      {"tcp_server_port", tcp_ep.port()},
      {"start_time", start_time.ToJavaScriptString()},
      {"timeout", static_cast<float>(timeout.count()) / 1000'000'000.0f},
  };
}

const LoggerPtr RemoteBranchInfo::logger_ =
    Logger::CreateStaticInternalLogger("Branch");

LocalBranchInfo::LocalBranchInfo() {
  pid = utils::GetProcessId();
  hostname = utils::GetHostname();
  start_time = utils::Timestamp::Now();
}

nlohmann::json LocalBranchInfo::ToJson() const {
  auto json = BranchInfo::ToJson();
  json["advertising_address"] = adv_ep.address().to_string();
  json["advertising_port"] = adv_ep.port();
  json["advertising_interval"] =
      static_cast<float>(adv_interval.count()) / 1000'000'000.0f;
  return json;
}

std::vector<char> LocalBranchInfo::MakeAdvertisingMessage() const {
  std::vector<char> msg = {'Y', 'O', 'G', 'I', 0};
  msg.push_back(api::kVersionMajor);
  msg.push_back(api::kVersionMinor);
  utils::Serialize(&msg, uuid);
  utils::Serialize(&msg, tcp_ep.port());
  return msg;
}

std::vector<char> LocalBranchInfo::MakeInfoMessage() const {
  std::vector<char> buffer;
  utils::Serialize(&buffer, name);
  utils::Serialize(&buffer, description);
  utils::Serialize(&buffer, net_name);
  utils::Serialize(&buffer, path);
  utils::Serialize(&buffer, hostname);
  utils::Serialize(&buffer, pid);
  utils::Serialize(&buffer, start_time);
  utils::Serialize(&buffer, timeout);
  utils::Serialize(&buffer, adv_interval);

  auto msg = MakeAdvertisingMessage();
  utils::Serialize(&msg, buffer.size());
  msg.insert(msg.end(), buffer.begin(), buffer.end());

  return msg;
}

std::shared_ptr<RemoteBranchInfo> RemoteBranchInfo::Create(
    const boost::uuids::uuid& uuid,
    const boost::asio::ip::tcp::endpoint& tcp_ep) {
  auto info = std::make_shared<RemoteBranchInfo>();
  info->uuid = uuid;
  info->tcp_ep = tcp_ep;

  return info;
}

std::shared_ptr<RemoteBranchInfo>
RemoteBranchInfo::CreateFromAdvertisingMessage(
    const std::vector<char>& msg, utils::TimedTcpSocketPtr socket) {
  YOGI_ASSERT(!CheckAdvertisingMessageValidity(msg));

  auto it = msg.cbegin() + GetAdvertisingMessageHeaderSize();

  auto info = std::make_shared<RemoteBranchInfo>();
  if (!DeserializeField(&info->uuid, msg, &it)) {
    YOGI_ASSERT(false);
    return {};
  }

  unsigned short port;
  if (!DeserializeField(&port, msg, &it)) {
    YOGI_ASSERT(false);
    return {};
  }

  info->tcp_ep.port(port);
  info->tcp_ep.address(socket->GetRemoteEndpoint().address());
  info->socket = socket;

  return info;
}

bool RemoteBranchInfo::DeserializeInfoMessageBody(
    const std::vector<char>& msg) {
  auto it = msg.begin();
  if (!DeserializeField(&name, msg, &it)) return false;
  if (!DeserializeField(&description, msg, &it)) return false;
  if (!DeserializeField(&net_name, msg, &it)) return false;
  if (!DeserializeField(&path, msg, &it)) return false;
  if (!DeserializeField(&hostname, msg, &it)) return false;
  if (!DeserializeField(&pid, msg, &it)) return false;
  if (!DeserializeField(&start_time, msg, &it)) return false;
  if (!DeserializeField(&timeout, msg, &it)) return false;
  if (!DeserializeField(&this->adv_interval, msg, &it)) return false;

  return true;
}

nlohmann::json RemoteBranchInfo::ToJson() const {
  auto json = BranchInfo::ToJson();
  json["connected"] = connected;
  json["last_connected"] = last_connected.ToJavaScriptString();
  json["last_disconnected"] = last_disconnected.ToJavaScriptString();
  json["last_activity"] = last_activity.ToJavaScriptString();
  json["last_error"] = last_error.error_code();
  return json;
}

}  // namespace detail
}  // namespace objects

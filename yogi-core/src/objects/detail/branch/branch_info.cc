#include "branch_info.h"
#include "../../../api/constants.h"
#include "../../../utils/system.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/endian/arithmetic.hpp>

namespace objects {
namespace detail {

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
      {"retry_time", static_cast<float>(retry_time.count()) / 1000'000'000.0f},
  };
}

LocalBranchInfo::LocalBranchInfo() {
  pid = utils::GetProcessId();
  hostname = utils::GetHostname();
  start_time = utils::Timestamp::Now();
}

nlohmann::json LocalBranchInfo::ToJson() const {
  auto json = BranchInfo::ToJson();
  json["advertising_address"] = adv_ep.address().to_string();
  json["advertising_port"] = adv_ep.port();
  json["advertising_interval"] = static_cast<float>(adv_interval.count()) / 1000'000'000.0f;
  return json;
}

std::vector<char> LocalBranchInfo::MakeAdvertisingMessage() const {
  std::vector<char> msg = {'Y', 'O', 'G', 'I', 0};
  msg.push_back(api::kVersionMajor);
  msg.push_back(api::kVersionMinor);
  msg.insert(msg.end(), uuid.begin(), uuid.end());
  boost::endian::big_uint16_t port = tcp_ep.port();
  msg.resize(msg.size() + 2);
  std::memcpy(msg.data() + msg.size() - 2, &port, 2);
  return msg;
}

std::vector<char> LocalBranchInfo::MakeInfoMessage() const {
  return {}; // TODO
}

nlohmann::json RemoteBranchInfo::ToJson() const {
  auto json = BranchInfo::ToJson();
  json["connected"] = connected;
  json["last_connected"] = last_connected.ToJavaScriptString();
  json["last_disconnected"] = last_disconnected.ToJavaScriptString();
  json["last_activity"] = last_activity.ToJavaScriptString();
  json["last_error"] = last_error;
  return json;
}

}  // namespace detail
}  // namespace objects

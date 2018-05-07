#include "branch_info.h"
#include "../../../utils/system.h"

namespace objects {
namespace detail {

BranchInfo::BranchInfo(const boost::uuids::uuid& uuid) : uuid(uuid), pid(0) {}

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
      {"timeout", (float)timeout.count() / 1000'000'000.0f},
      {"retry_time", (float)retry_time.count() / 1000'000'000.0f},
  };
}

LocalBranchInfo::LocalBranchInfo(const boost::uuids::uuid& uuid)
    : BranchInfo(uuid) {
  pid = utils::GetProcessId();
  hostname = utils::GetHostname();
  start_time = utils::Timestamp::Now();
}

nlohmann::json LocalBranchInfo::ToJson() const {
  auto json = BranchInfo::ToJson();
  json["advertising_address"] = adv_ep.address().to_string();
  json["advertising_port"] = adv_ep.port();
  json["advertising_interval"] = (float)adv_interval.count() / 1000'000'000.0f;
  return json;
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

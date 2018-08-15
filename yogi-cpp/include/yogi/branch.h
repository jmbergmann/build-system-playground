#pragma once

#include "object.h"
#include "io.h"
#include "uuid.h"
#include "duration.h"
#include "timestamp.h"
#include "internal/library.h"
#include "internal/flags.h"
#include "internal/json.h"

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_BranchCreate,
                   (void** branch, void* context, const char* name,
                    const char* description, const char* netname,
                    const char* password, const char* path, const char* advaddr,
                    int advport, long long advint, long long timeout))
YOGI_DEFINE_API_FN(int, YOGI_BranchGetInfo,
                   (void* branch, void* uuid, char* json, int jsonsize))
YOGI_DEFINE_API_FN(int, YOGI_BranchGetConnectedBranches,
                   (void* branch, void* uuid, char* json, int jsonsize,
                    void (*fn)(int res, void* userarg), void* userarg))
YOGI_DEFINE_API_FN(int, YOGI_BranchAwaitEvent,
                   (void* branch, int events, void* uuid, char* json,
                    int jsonsize,
                    void (*fn)(int res, int event, int evres, void* userarg),
                    void* userarg))
YOGI_DEFINE_API_FN(int, YOGI_BranchCancelAwaitEvent, (void* branch))

/// Branch events.
///
/// Branch events are events that can be observed on a branch.
enum class BranchEvents {
  /// No events.
  kNone = 0,

  /// A new branch has been discovered.
  kBranchDiscovered = (1 << 0),

  /// Querying a new branch for information finished.
  kBranchQueried = (1 << 1),

  /// Connecting to a branch finished.
  kConnectFinished = (1 << 2),

  /// The connection to a branch was lost.
  kConnectionLost = (1 << 3),

  /// Combination of all flags.
  kAll =
      kBranchDiscovered | kBranchQueried | kConnectFinished | kConnectionLost,
};

YOGI_DEFINE_FLAG_OPERATORS(BranchEvents)

template <>
inline std::string ToString<BranchEvents>(const BranchEvents& events) {
  switch (events) {
    YOGI_TO_STRING_ENUM_CASE(BranchEvents, kNone)
    YOGI_TO_STRING_ENUM_CASE(BranchEvents, kBranchDiscovered)
    YOGI_TO_STRING_ENUM_CASE(BranchEvents, kBranchQueried)
    YOGI_TO_STRING_ENUM_CASE(BranchEvents, kConnectFinished)
    YOGI_TO_STRING_ENUM_CASE(BranchEvents, kConnectionLost)
    YOGI_TO_STRING_ENUM_CASE(BranchEvents, kAll)
  }

  std::string s;
  YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kBranchDiscovered)
  YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kBranchQueried)
  YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kConnectFinished)
  YOGI_TO_STRING_FLAG_APPENDER(events, BranchEvents, kConnectionLost)
  return s.substr(3);
}

/// Information about about a branch.
class BranchInfo {
 public:
  /// Constructor.
  ///
  /// \tparam String String type.
  ///
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  template <typename String>
  BranchInfo(const Uuid& uuid, String json)
      : uuid_(uuid),
        json_(internal::StringToCoreString(json)),
        name_(internal::ExtractStringFromJson(json_, "name")),
        description_(internal::ExtractStringFromJson(json_, "description")),
        net_name_(internal::ExtractStringFromJson(json_, "net_name")),
        path_(internal::ExtractStringFromJson(json_, "path")),
        hostname_(internal::ExtractStringFromJson(json_, "hostname")),
        pid_(internal::ExtractIntFromJson(json_, "pid")),
        adv_internal_(
            internal::ExtractDurationFromJson(json_, "advertising_interval")),
        tcp_server_address_(
            internal::ExtractStringFromJson(json_, "tcp_server_address")),
        tcp_server_port_(
            internal::ExtractIntFromJson(json_, "tcp_server_port")),
        start_time_(internal::ExtractTimestampFromJson(json_, "start_time")),
        timeout_(internal::ExtractDurationFromJson(json_, "timeout")) {}

  /// UUID of the branch.
  const Uuid& GetUuid() const { return uuid_; }

  /// Name of the branch.
  const std::string& GetName() const { return name_; }

  /// Description of the branch.
  const std::string& GetDescription() const { return description_; }

  /// Name of the network.
  const std::string& GetNetName() const { return net_name_; }

  /// Path of the branch.
  const std::string& GetPath() const { return path_; }

  /// The machine's hostname..
  const std::string& GetHostname() const { return hostname_; }

  /// ID of the process.
  int GetPid() const { return pid_; }

  /// Advertising interval.
  const Duration& GetAdvertisingInterval() const { return adv_interval_; }

  /// Address of the TCP server for incoming connections.
  const std::string& GetTcpServerAddress() const { return tcp_server_address_; }

  /// Listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return tcp_server_port_; }

  /// Time when the branch was started (UTC time).
  const Timestamp& GetStartTime() const { return start_time_; }

  /// Connection timeout.
  const Duration& GetTimeout() const { return timeout_; }

  /// Returns the branch information as JSON-encoded string.
  ///
  /// \returns Branch information as JSON-encoded string.
  const std::string& ToString() const { return json_; }

 private:
  const Uuid uuid_;
  const std::string json_;
  const std::string name_;
  const std::string description_;
  const std::string net_name_;
  const std::string path_;
  const std::string hostname_;
  const int pid_;
  const Duration adv_interval_;
  const std::string tcp_server_address_;
  const int tcp_server_port_;
  const Timestamp start_time_;
  const Duration timeout_;
};

/// Information about a remote branch.
class RemoteBranchInfo : public BranchInfo {
 public:
  using BranchInfo::BranchInfo;
};

/// Information about a local branch.
class LocalBranchInfo : public BranchInfo {
 public:
  /// Constructor.
  ///
  /// \tparam String String type.
  ///
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  template <typename String>
  LocalBranchInfo(const Uuid& uuid, String json)
      : BranchInfo(uuid, json),
        adv_address_(internal::ExtractFromJson<std::string>(
            ToString(), "advertising_address")),
        adv_port_(
            internal::ExtractIntFromJson(ToString(), "advertising_port")) {}

  /// Advertising IP address.
  const std::string& GetAdvertisingAddress() const { return adv_address_; }

  /// Advertising port.
  int GetAdvertisingPort() const { return adv_port_; }

 private:
  const std::string adv_address_;
  const int adv_port_;
};

/// Information associated with a branch event.
class BranchEventInfo {};

/// Information associated with the BranchDiscovered branch event.
class BranchDiscoveredEventInfo : public BranchEventInfo {};

/// Information associated with the BranchQueried branch event.
class BranchQueriedEventInfo : public BranchEventInfo {};

/// Information associated with the ConnectFinished branch event.
class ConnectFinishedEventInfo : public BranchEventInfo {};

/// Information associated with the ConnectionLost branch event.
class ConnectionLostEventInfo : public BranchEventInfo {};

///
class Branch : public Object {};

}  // namespace yogi

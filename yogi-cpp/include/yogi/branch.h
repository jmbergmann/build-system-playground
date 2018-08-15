#pragma once

#include "object.h"
#include "context.h"
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
  BranchInfo(const Uuid& uuid, String&& json)
      : uuid_(uuid),
        json_(json),
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

  /// Returns the UUID of the branch.
  ///
  /// \returns UUID of the branch.
  const Uuid& GetUuid() const { return uuid_; }

  /// Returns the name of the branch.
  ///
  /// \returns The name of the branch.
  const std::string& GetName() const { return name_; }

  /// Returns the description of the branch.
  ///
  /// \returns The description of the branch.
  const std::string& GetDescription() const { return description_; }

  /// Returns the name of the network.
  ///
  /// \returns The name of the network.
  const std::string& GetNetName() const { return net_name_; }

  /// Returns the  path of the branch.
  ///
  /// \returns The  path of the branch.
  const std::string& GetPath() const { return path_; }

  /// Returns the machine's hostname.
  ///
  /// \returns The machine's hostname.
  const std::string& GetHostname() const { return hostname_; }

  /// Returns the ID of the process.
  ///
  /// \returns The ID of the process.
  int GetPid() const { return pid_; }

  /// Returns the advertising interval.
  ///
  /// \returns The advertising interval.
  const Duration& GetAdvertisingInterval() const { return adv_interval_; }

  /// Returns the address of the TCP server for incoming connections.
  ///
  /// \returns The address of the TCP server for incoming connections.
  const std::string& GetTcpServerAddress() const { return tcp_server_address_; }

  /// Returns the listening port of the TCP server for incoming connections.
  ///
  /// \returns The listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return tcp_server_port_; }

  /// Returns the time when the branch was started (UTC time).
  ///
  /// \returns The time when the branch was started (UTC time).
  const Timestamp& GetStartTime() const { return start_time_; }

  /// Returns the connection timeout.
  ///
  /// \returns The connection timeout.
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
  LocalBranchInfo(const Uuid& uuid, String&& json)
      : BranchInfo(uuid, json),
        adv_address_(internal::ExtractFromJson<std::string>(
            ToString(), "advertising_address")),
        adv_port_(
            internal::ExtractIntFromJson(ToString(), "advertising_port")) {}

  /// Advertising IP address.
  ///
  /// \return The advertising IP address.
  const std::string& GetAdvertisingAddress() const { return adv_address_; }

  /// Advertising port.
  ///
  /// \return The advertising port.
  int GetAdvertisingPort() const { return adv_port_; }

 private:
  const std::string adv_address_;
  const int adv_port_;
};

/// Information associated with a branch event.
class BranchEventInfo {
 public:
  /// Constructor.
  ///
  /// \tparam String String type.
  ///
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  template <typename String>
  BranchEventInfo(const Uuid& uuid, String&& json) : uuid_(uuid), json_(json) {}

  /// Returns the UUID of the branch.
  ///
  /// \returns UUID of the branch.
  const Uuid& GetUuid() const { return uuid_; }

  /// Returns the event information as JSON-encoded string.
  ///
  /// \returns Event information as JSON-encoded string.
  const std::string& ToString() const { return json_; }

 private:
  const Uuid uuid_;
  const std::string json_;
};

/// Information associated with the BranchDiscovered branch event.
class BranchDiscoveredEventInfo : public BranchEventInfo {
 public:
  /// Constructor.
  ///
  /// \tparam String String type.
  ///
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  template <typename String>
  BranchDiscoveredEventInfo(const Uuid& uuid, String&& json)
      : BranchEventInfo(uuid, json),
        tcp_server_address_(
            internal::ExtractStringFromJson(json_, "tcp_server_address")),
        tcp_server_port_(
            internal::ExtractIntFromJson(json_, "tcp_server_port")) {}

  /// Returns the address of the TCP server for incoming connections.
  ///
  /// \returns The address of the TCP server for incoming connections.
  const std::string& GetTcpServerAddress() const { return tcp_server_address_; }

  /// Returns the listening port of the TCP server for incoming connections.
  ///
  /// \returns The listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return tcp_server_port_; }

 private:
  const std::string tcp_server_address_;
  const int tcp_server_port_;
};

/// Information associated with the BranchQueried branch event.
class BranchQueriedEventInfo : public BranchEventInfo {
 public:
  /// Constructor.
  ///
  /// \tparam String String type.
  ///
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  template <typename String>
  BranchQueriedEventInfo(const Uuid& uuid, String&& json)
      : BranchEventInfo(uuid, json), info_(RemoteBranchInfo(uuid, json)) {}

  /// Returns the name of the branch.
  ///
  /// \returns The name of the branch.
  const std::string& GetName() const { return info_.GetName(); }

  /// Returns the description of the branch.
  ///
  /// \returns The description of the branch.
  const std::string& GetDescription() const { return info_.GetDescription(); }

  /// Returns the name of the network.
  ///
  /// \returns The name of the network.
  const std::string& GetNetName() const { return info_.GetNetName(); }

  /// Returns the  path of the branch.
  ///
  /// \returns The  path of the branch.
  const std::string& GetPath() const { return info_.GetPath(); }

  /// Returns the machine's hostname.
  ///
  /// \returns The machine's hostname.
  const std::string& GetHostname() const { return info_.GetHostname(); }

  /// Returns the ID of the process.
  ///
  /// \returns The ID of the process.
  int GetPid() const { return info_.GetPid(); }

  /// Returns the advertising interval.
  ///
  /// \returns The advertising interval.
  const Duration& GetAdvertisingInterval() const {
    return info_.GetAdvertisingInterval();
  }

  /// Returns the address of the TCP server for incoming connections.
  ///
  /// \returns The address of the TCP server for incoming connections.
  const std::string& GetTcpServerAddress() const {
    return info_.GetTcpServerAddress();
  }

  /// Returns the listening port of the TCP server for incoming connections.
  ///
  /// \returns The listening port of the TCP server for incoming connections.
  int GetTcpServerPort() const { return info_.GetTcpServerPort(); }

  /// Returns the time when the branch was started (UTC time).
  ///
  /// \returns The time when the branch was started (UTC time).
  const Timestamp& GetStartTime() const { return info_.GetStartTime(); }

  /// Returns the connection timeout.
  ///
  /// \returns The connection timeout.
  const Duration& GetTimeout() const { return info_.GetTimeout(); }

  /// Converts the event information to a RemoteBranchInfo object.
  ///
  /// \returns The converted RemoteBranchInfo object.
  const RemoteBranchInfo& ToRemoteBranchInfo() const { return info_; }

 private:
  const RemoteBranchInfo info_;
};

/// Information associated with the ConnectFinished branch event.
class ConnectFinishedEventInfo : public BranchEventInfo {
 public:
  using BranchEventInfo::BranchEventInfo;
};

/// Information associated with the ConnectionLost branch event.
class ConnectionLostEventInfo : public BranchEventInfo {
 public:
  using BranchEventInfo::BranchEventInfo;
};

// class Branch;
// typedef std::shared_ptr<Branch> BranchPtr;

// /// Entry point into a Yogi network.
// ///
// /// A branch represents an entry point into a YOGI network. It advertises
// /// itself via IP broadcasts/multicasts with its unique ID and information
// /// required for establishing a connection. If a branch detects other branches
// /// on the network, it connects to them via TCP to retrieve further
// /// information such as their name, description and network name. If the
// /// network names match, two branches attempt to authenticate with each other
// /// by securely comparing passwords. Once authentication succeeds and there is
// /// no other known branch with the same path then the branches can actively
// /// communicate as part of the Yogi network.
// ///
// /// Note: Even though the authentication process via passwords is done in a
// ///       secure manner, any further communication is done in plain text.
// class Branch : public Object {
//  public:
//   /// Creates the branch.
//   ///
//   /// Advertising and establishing connections can be limited to certain
//   /// network interfaces via the interface parameter. The default is to use
//   /// all available interfaces.
//   ///
//   /// Setting the advint parameter to infinity prevents the branch from
//   /// actively participating in the Yogi network, i.e. the branch will not
//   /// advertise itself and it will not authenticate in order to join a
//   /// network. However, the branch will temporarily connect to other
//   /// branches in order to obtain more detailed information such as name,
//   /// description, network name and so on. This is useful for obtaining
//   /// information about active branches without actually becoming part of
//   /// the Yogi network.
//   template <typename NameString, typename DescriptionString = char*,
//             typename NetnameString = char*, typename PasswordString = char*,
//             typename PathString = char*, typename AdvaddrString = char*>
//   Branch(ContextPtr context, NameString&& name,
//          DescriptionString&& description = nullptr,
//          NetnameString&& netname = nullptr, PasswordString&& password = nullptr,
//          PathString&& path = nullptr, AdvaddrString&& advaddr = nullptr,
//          int advport = 0, const Duration& advint = Duration::kZero,
//          const Duration& timeout = Duration::kZero) {}

//  private:
// };

}  // namespace yogi

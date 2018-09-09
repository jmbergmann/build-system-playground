#ifndef YOGI_BRANCH_H
#define YOGI_BRANCH_H

#include "object.h"
#include "context.h"
#include "io.h"
#include "uuid.h"
#include "duration.h"
#include "timestamp.h"
#include "constants.h"
#include "internal/library.h"
#include "internal/flags.h"
#include "internal/json.h"
#include "internal/query_string.h"
#include "internal/json_view.h"
#include "internal/string_view.h"

#include <unordered_map>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_BranchCreate,
                   (void** branch, void* context, const char* props,
                    const char* section, char* err, int errsize))
YOGI_DEFINE_API_FN(int, YOGI_BranchGetInfo,
                   (void* branch, void* uuid, char* json, int jsonsize))
YOGI_DEFINE_API_FN(int, YOGI_BranchGetConnectedBranches,
                   (void* branch, void* uuid, char* json, int jsonsize,
                    void (*fn)(int res, void* userarg), void* userarg))
YOGI_DEFINE_API_FN(int, YOGI_BranchAwaitEvent,
                   (void* branch, int events, void* uuid, char* json,
                    int jsonsize,
                    void (*fn)(int res, int event, int ev_res, void* userarg),
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
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  BranchInfo(const Uuid& uuid, internal::JsonView json)
      : uuid_(uuid),
        json_(json),
        name_(internal::ExtractStringFromJson(json_, "name")),
        description_(internal::ExtractStringFromJson(json_, "description")),
        network_name_(internal::ExtractStringFromJson(json_, "network_name")),
        path_(internal::ExtractStringFromJson(json_, "path")),
        hostname_(internal::ExtractStringFromJson(json_, "hostname")),
        pid_(internal::ExtractIntFromJson(json_, "pid")),
        adv_interval_(
            internal::ExtractDurationFromJson(json_, "advertising_interval")),
        tcp_server_address_(
            internal::ExtractStringFromJson(json_, "tcp_server_address")),
        tcp_server_port_(
            internal::ExtractIntFromJson(json_, "tcp_server_port")),
        start_time_(internal::ExtractTimestampFromJson(json_, "start_time")),
        timeout_(internal::ExtractDurationFromJson(json_, "timeout")),
        ghost_mode_(internal::ExtractBoolFromJson(json_, "ghost_mode")) {}

  virtual ~BranchInfo() {}

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
  const std::string& GetNetworkName() const { return network_name_; }

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

  /// Returns true if the branch is in ghost mode.
  ///
  /// \return True if the branch is in ghost mode.
  bool GetGhostMode() const { return ghost_mode_; }

  /// Returns the branch information as JSON-encoded string.
  ///
  /// \returns Branch information as JSON-encoded string.
  const std::string& ToString() const { return json_; }

 private:
  const Uuid uuid_;
  const std::string json_;
  const std::string name_;
  const std::string description_;
  const std::string network_name_;
  const std::string path_;
  const std::string hostname_;
  const int pid_;
  const Duration adv_interval_;
  const std::string tcp_server_address_;
  const int tcp_server_port_;
  const Timestamp start_time_;
  const Duration timeout_;
  const bool ghost_mode_;
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
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  LocalBranchInfo(const Uuid& uuid, internal::JsonView json)
      : BranchInfo(uuid, json),
        adv_address_(
            internal::ExtractStringFromJson(ToString(), "advertising_address")),
        adv_port_(
            internal::ExtractIntFromJson(ToString(), "advertising_port")) {}

  /// Advertising IP address.
  ///
  /// \returns The advertising IP address.
  const std::string& GetAdvertisingAddress() const { return adv_address_; }

  /// Advertising port.
  ///
  /// \returns The advertising port.
  int GetAdvertisingPort() const { return adv_port_; }

 private:
  const std::string adv_address_;
  const int adv_port_;
};

/// Information associated with a branch event.
class BranchEventInfo {
 public:
  /// Constructor for an empty event info object.
  BranchEventInfo() : uuid_{}, json_("{}") {}

  /// Constructor.
  ///
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  BranchEventInfo(const Uuid& uuid, internal::JsonView json)
      : uuid_(uuid), json_(json) {}

  virtual ~BranchEventInfo() {}

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
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  BranchDiscoveredEventInfo(const Uuid& uuid, internal::JsonView json)
      : BranchEventInfo(uuid, json),
        tcp_server_address_(
            internal::ExtractStringFromJson(ToString(), "tcp_server_address")),
        tcp_server_port_(
            internal::ExtractIntFromJson(ToString(), "tcp_server_port")) {}

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
  /// \param uuid UUID of the branch.
  /// \param json JSON string to parse.
  BranchQueriedEventInfo(const Uuid& uuid, internal::JsonView json)
      : BranchEventInfo(uuid, json),
        info_(RemoteBranchInfo(uuid, ToString())) {}

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
  const std::string& GetNetworkName() const { return info_.GetNetworkName(); }

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

  /// Returns true if the branch is in ghost mode.
  ///
  /// \return True if the branch is in ghost mode.
  bool GetGhostMode() const { return info_.GetGhostMode(); }

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

class Branch;
using BranchPtr = std::shared_ptr<Branch>;

/// Entry point into a Yogi network.
///
/// A branch represents an entry point into a YOGI network. It advertises
/// itself via IP broadcasts/multicasts with its unique ID and information
/// required for establishing a connection. If a branch detects other branches
/// on the network, it connects to them via TCP to retrieve further
/// information such as their name, description and network name. If the
/// network names match, two branches attempt to authenticate with each other
/// by securely comparing passwords. Once authentication succeeds and there is
/// no other known branch with the same path then the branches can actively
/// communicate as part of the Yogi network.
///
/// Note: Even though the authentication process via passwords is done in a
///       secure manner, any further communication is done in plain text.
class Branch : public ObjectT<Branch> {
 public:
  using AwaitEventFn =
      std::function<void(const Result& res, BranchEvents event,
                         const Result& ev_res, BranchEventInfo& info)>;

  /// Creates a branch.
  ///
  /// The branch is configured via the \p props parameter. The supplied JSON
  /// must have the following structure:
  ///
  /// \code
  ///    {
  ///      "name":                 "Fan Controller",
  ///      "description":          "Controls a fan via PWM",
  ///      "path":                 "/Cooling System/Fan Controller",
  ///      "network_name":         "Hardware Control",
  ///      "network_password":     "secret",
  ///      "advertising_address":  "ff31::8000:2439",
  ///      "advertising_port":     13531,
  ///      "advertising_interval": 1.0,
  ///      "timeout":              3.0,
  ///      "ghost_mode":           false
  ///    }
  /// \endcode
  ///
  /// All of the properties are optional and if unspecified (or set to _null_),
  /// their respective default values will be used (see \ref CV). The properties
  /// have the following meaning:
  ///  - *name*: Name of the branch (default: PID\@hostname without the
  ///    backslash).
  ///  - *description*: Description of the branch.
  ///  - *path*: Path of the branch in the network (default: /name where name is
  ///    the name of the branch). Must start with a slash.
  ///  - *network_name*: Name of the network to join (default: the machine's
  ///    hostname).
  ///  - *network_password*: Password for the network (default: no password)
  ///  - *advertising_address*: Multicast address to use for advertising, e.g.
  ///    239.255.0.1 for IPv4 or ff31::8000:1234 for IPv6.
  ///  - *advertising_port*: Port to use for advertising.
  ///  - *advertising_interval*: Time between advertising messages. Must be at
  ///    least 1 ms.
  ///  - *ghost_mode*: Set to true to activate ghost mode (default: false).
  ///
  /// Advertising and establishing connections can be limited to certain network
  /// interfaces via the _interface_ property. The default is to use all
  /// available interfaces.
  ///
  /// Setting the _ghost_mode_ property to _true_ prevents the branch from
  /// actively participating in the Yogi network, i.e. the branch will not
  /// advertise itself and it will not authenticate in order to join a network.
  /// However, the branch will temporarily connect to other branches in order to
  /// obtain more detailed information such as name, description, network name
  /// and so on. This is useful for obtaining information about active branches
  /// without actually becoming part of the Yogi network.
  ///
  /// \param context Context to use
  /// \param props   Branch properties as serialized JSON
  /// \param section Section in \p props to use instead of the root section;
  ///                syntax is JSON pointer (RFC 6901)
  ///
  /// \returns The created branch.
  static BranchPtr Create(ContextPtr context, internal::JsonView props = {},
                          internal::StringView section = {}) {
    return BranchPtr(new Branch(context, props, section));
  }

  /// Returns information about the local branch.
  ///
  /// \returns Information about the local branch.
  const LocalBranchInfo& GetInfo() const { return info_; }

  /// Returns the UUID of the branch.
  ///
  /// \returns UUID of the branch.
  const Uuid& GetUuid() const { return info_.GetUuid(); }

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
  const std::string& GetNetworkName() const { return info_.GetNetworkName(); }

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

  /// Returns true if the branch is in ghost mode.
  ///
  /// \return True if the branch is in ghost mode.
  bool GetGhostMode() const { return info_.GetGhostMode(); }

  /// Advertising IP address.
  ///
  /// \returns The advertising IP address.
  const std::string& GetAdvertisingAddress() const {
    return info_.GetAdvertisingAddress();
  }

  /// Advertising port.
  ///
  /// \returns The advertising port.
  int GetAdvertisingPort() const { return info_.GetAdvertisingPort(); }

  /// Retrieves information about all connected remote branches.
  ///
  /// \returns A map mapping the UUID of each connected remote branch to a
  ///          RemoteBranchInfo object with detailed information about the
  ///          branch.
  std::unordered_map<Uuid, RemoteBranchInfo> GetConnectedBranches() const {
    struct CallbackData {
      Uuid uuid;
      const char* str;
      std::unordered_map<Uuid, RemoteBranchInfo> branches;
    } data;

    internal::QueryString([&](auto str, auto size) {
      data.str = str;
      data.branches.clear();

      return internal::YOGI_BranchGetConnectedBranches(
          this->GetHandle(), &data.uuid, str, size,
          [](int res, void* userarg) {
            if (res == static_cast<int>(ErrorCode::kOk)) {
              auto data = static_cast<CallbackData*>(userarg);
              data->branches.emplace(std::make_pair(
                  data->uuid, RemoteBranchInfo(data->uuid, data->str)));
            }
          },
          &data);
    });

    return data.branches;
  }

  /// Waits for a branch event to occur.
  ///
  /// This function will register the handler fn to be executed once one of the
  /// given branch events occurs. If this function is called while a previous
  /// wait operation is still active then the previous opertion will be
  /// canceled, i.e. the handler \p fn for the previous operation will be called
  /// with a cancellation error.
  ///
  /// If successful, the event information passed to the handler function \p fn
  /// contains at least the UUID of the remote branch.
  ///
  /// In case that the internal buffer for reading the event information is too
  /// small, fn will be called with the corresponding error and the event
  /// information is lost. You can set the size of this buffer via the
  /// \p buffer_size parameter.
  ///
  /// \param events      Events to observe.
  /// \param fn          Handler function to call.
  /// \param buffer_size Size of the internal buffer for reading event
  ///                    information.
  void AwaitEvent(BranchEvents events, AwaitEventFn fn,
                  int buffer_size = 1024) {
    struct CallbackData {
      AwaitEventFn fn;
      Uuid uuid;
      std::vector<char> json;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;
    data->json.resize(static_cast<std::size_t>(buffer_size));

    int res = internal::YOGI_BranchAwaitEvent(
        GetHandle(), static_cast<int>(events), &data->uuid, data->json.data(),
        buffer_size,
        [](int res, int event, int ev_res, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));

          if (!data->fn) {
            return;
          }

          auto be = static_cast<BranchEvents>(event);

          if (Result(res) && be != BranchEvents::kNone) {
            switch (be) {
              case BranchEvents::kBranchDiscovered:
                CallAwaitEventFn<BranchDiscoveredEventInfo>(res, be, ev_res,
                                                            data);
                break;

              case BranchEvents::kBranchQueried:
                CallAwaitEventFn<BranchQueriedEventInfo>(res, be, ev_res, data);
                break;

              case BranchEvents::kConnectFinished:
                CallAwaitEventFn<ConnectFinishedEventInfo>(res, be, ev_res,
                                                           data);
                break;

              case BranchEvents::kConnectionLost:
                CallAwaitEventFn<ConnectionLostEventInfo>(res, be, ev_res,
                                                          data);
                break;

              default: {
                bool should_never_get_here = false;
                assert(should_never_get_here);
              }
            }
          } else {
            CallAwaitEventFn<BranchEventInfo>(res, be, ev_res, data);
          }
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();
  }

  /// Cancels waiting for a branch event.
  ///
  /// Calling this function will cause the handler registered via AwaitEvent()
  /// to be called with a cancellation error.
  void CancelAwaitEvent() {
    int res = internal::YOGI_BranchCancelAwaitEvent(GetHandle());
    internal::CheckErrorCode(res);
  }

 private:
  Branch(ContextPtr context, internal::JsonView props,
         internal::StringView section)
      : ObjectT(internal::CallApiCreateWithDescriptiveErrorCode(
                    internal::YOGI_BranchCreate, GetForeignHandle(context),
                    props, section),
                {context}),
        info_(QueryInfo()) {}

  LocalBranchInfo QueryInfo() {
    Uuid uuid;
    auto json = internal::QueryString([&](auto str, auto size) {
      return internal::YOGI_BranchGetInfo(this->GetHandle(), &uuid, str, size);
    });

    return LocalBranchInfo(uuid, json);
  }

  template <typename EventInfo, typename CallbackData>
  static void CallAwaitEventFn(int res, BranchEvents be, int ev_res,
                               CallbackData&& data) {
    internal::WithErrorCodeToResult(res, [&](const auto& result) {
      if (result) {
        EventInfo info(data->uuid, data->json.data());
        internal::WithErrorCodeToResult(ev_res, [&](const auto& ev_result) {
          data->fn(result, be, ev_result, info);
        });
      } else {
        BranchEventInfo info;
        internal::WithErrorCodeToResult(ev_res, [&](const auto& ev_result) {
          data->fn(result, be, ev_result, info);
        });
      }
    });
  }

  const LocalBranchInfo info_;
};

}  // namespace yogi

#endif  // YOGI_BRANCH_H

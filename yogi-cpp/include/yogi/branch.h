#pragma once

#include "object.h"
#include "io.h"
#include "internal/library.h"
#include "internal/flags.h"

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
  kAll = kBranchDiscovered | kBranchQueried | kConnectFinished | kConnectionLost,
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
};

/// Information about a remote branch.
class RemoteBranchInfo : public BranchInfo {

};

/// Information about a local branch.
class LocalBranchInfo : public BranchInfo {

};

/// Information associated with a branch event.
class BranchEventInfo {

};

/// Information associated with the BranchDiscovered branch event.
class BranchDiscoveredEventInfo : public BranchEventInfo {

};

/// Information associated with the BranchQueried branch event.
class BranchQueriedEventInfo : public BranchEventInfo {

};

/// Information associated with the ConnectFinished branch event.
class ConnectFinishedEventInfo : public BranchEventInfo {

};

/// Information associated with the ConnectionLost branch event.
class ConnectionLostEventInfo : public BranchEventInfo {

};

///
class Branch : public Object {};

}  // namespace yogi

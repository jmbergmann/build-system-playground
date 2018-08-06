#pragma once

#include "../config.h"

namespace api {

// clang-format off
#define SCC static constexpr const
SCC char*     kVersionNumber                 = YOGI_HDR_VERSION;
SCC int       kVersionMajor                  = YOGI_HDR_VERSION_MAJOR;
SCC int       kVersionMinor                  = YOGI_HDR_VERSION_MINOR;
SCC int       kVersionPatch                  = YOGI_HDR_VERSION_PATCH;
SCC char*     kDefaultAdvAddress             = "ff31::8000:2439";
SCC int       kDefaultAdvPort                = 13531;
SCC long long kDefaultAdvInterval            = 1'000'000'000;
SCC long long kDefaultConnectionTimeout      = 3'000'000'000;
SCC int       kDefaultLoggerVerbosity        = YOGI_VB_INFO;
SCC char*     kDefaultLogTimeFormat          = "%F %T.%3";
SCC char*     kDefaultLogFormat              = "$t [T$T] $<$s $c: $m$>";
SCC int       kMaxMessageSize                = 100'000;
SCC char*     kDefaultTimeFormat             = "%FT%T.%3Z";
SCC char*     kDefaultInfiniteDurationString = "%-inf";
SCC char*     kDefaultDurationFormat         = "%-%dd %T.%3%6%9";
SCC char*     kDefaultInvalidHandleString    = "INVALID";
SCC char*     kDefaultObjectFormat           = "$T [$x]";
#undef SCC
// clang-format on

void GetConstant(void* dest, int constant);

}  // namespace api

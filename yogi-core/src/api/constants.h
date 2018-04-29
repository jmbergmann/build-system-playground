#pragma once

#include "../config.h"

namespace api {

#define SCC static constexpr const
SCC char* kVersionNumber       = YOGI_HDR_VERSION;
SCC int   kVersionMajor        = YOGI_HDR_VERSION_MAJOR;
SCC int   kVersionMinor        = YOGI_HDR_VERSION_MINOR;
SCC int   kVersionPatch        = YOGI_HDR_VERSION_PATCH;
SCC char* kDefaultAdvAddress   = "ff31::8000:2439";
SCC int   kDefaultAdvPort      = 13531;
SCC int   kDefaultAdvInterval  = 1000;
SCC int   kDefaultLogVerbosity = YOGI_VB_INFO;
SCC char* kDefaultLogFormat    = "%Y-%m-%d %T.%3 [T%t] %V %c: %X";
#undef SCC

void GetConstant(void* dest, int constant);

} // namespace api

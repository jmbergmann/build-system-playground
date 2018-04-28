#pragma once

#include "../config.h"

namespace api {

static constexpr const char* kVersionNumber      = YOGI_HDR_VERSION;
static constexpr const int   kVersionMajor       = YOGI_HDR_VERSION_MAJOR;
static constexpr const int   kVersionMinor       = YOGI_HDR_VERSION_MINOR;
static constexpr const int   kVersionPatch       = YOGI_HDR_VERSION_PATCH;
static constexpr const char* kDefaultAdvAddress  = "ff31::8000:2439";
static constexpr const int   kDefaultAdvPort     = 13531;
static constexpr const int   kDefaultAdvInterval = 1000;
static constexpr const char* kDefaultTraceFormat = "%Y-%m-%d %T.%3 %V: %X";

void GetConstant(void* dest, int constant);

} // namespace api

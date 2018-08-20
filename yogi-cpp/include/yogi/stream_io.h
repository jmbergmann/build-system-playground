#pragma once

#include "io.h"
#include "errors.h"
#include "object.h"
#include "logging.h"
#include "branch.h"
#include "uuid.h"
#include "duration.h"
#include "timestamp.h"
#include "branch.h"
#include "signals.h"
#include "configuration.h"

#include <sstream>

#define YOGI_DEFINE_OSTREAM_OPERATOR(type)                                   \
  inline std::ostream& operator<<(std::ostream& os, const yogi::type& val) { \
    os << yogi::ToString(val);                                               \
    return os;                                                               \
  }

YOGI_DEFINE_OSTREAM_OPERATOR(ErrorCode)
YOGI_DEFINE_OSTREAM_OPERATOR(Result)
YOGI_DEFINE_OSTREAM_OPERATOR(Object)
YOGI_DEFINE_OSTREAM_OPERATOR(Verbosity)
YOGI_DEFINE_OSTREAM_OPERATOR(Stream)
YOGI_DEFINE_OSTREAM_OPERATOR(BranchEvents)
YOGI_DEFINE_OSTREAM_OPERATOR(Uuid)
YOGI_DEFINE_OSTREAM_OPERATOR(Duration)
YOGI_DEFINE_OSTREAM_OPERATOR(Timestamp)
YOGI_DEFINE_OSTREAM_OPERATOR(BranchInfo)
YOGI_DEFINE_OSTREAM_OPERATOR(BranchEventInfo)
YOGI_DEFINE_OSTREAM_OPERATOR(Signals)
YOGI_DEFINE_OSTREAM_OPERATOR(ConfigurationFlags)
YOGI_DEFINE_OSTREAM_OPERATOR(CommandLineOptions)

#undef YOGI_DEFINE_OSTREAM_OPERATOR

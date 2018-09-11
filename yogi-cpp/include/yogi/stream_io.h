/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef YOGI_STREAM_IO_H
#define YOGI_STREAM_IO_H

//! \file
//!
//! Stream-based printing of various Yogi types.

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

#endif  // YOGI_STREAM_IO_H

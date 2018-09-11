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

#ifndef YOGI_ERRORS_H
#define YOGI_ERRORS_H

#include "internal/library.h"
#include "string_view.h"
#include "io.h"

#include <string>
#include <cassert>
#include <type_traits>

namespace yogi {

YOGI_DEFINE_API_FN(const char*, YOGI_GetErrorString, (int err))

////////////////////////////////////////////////////////////////////////////////
/// Error codes
///
/// Yogi error codes indicating failures are always < 0. A human-readable
/// description can be obtained via the Result class.
////////////////////////////////////////////////////////////////////////////////
enum class ErrorCode {
  /// Operation completed successfully
  kOk = 0,

  /// Unknown internal error occured
  kUnknown = -1,

  /// The object is still being used by another object
  kObjectStillUsed = -2,

  /// Insufficient memory to complete the operation
  kBadAlloc = -3,

  /// Invalid parameter
  kInvalidParam = -4,

  /// Invalid Handle
  kInvalidHandle = -5,

  /// Object is of the wrong type
  kWrongObjectType = -6,

  /// The operation has been canceled
  kCanceled = -7,

  /// Operation failed because the object is busy
  kBusy = -8,

  /// The operation timed out
  kTimeout = -9,

  /// The timer has not been started or already expired
  kTimerExpired = -10,

  /// The supplied buffer is too small
  kBufferTooSmall = -11,

  /// Could not open a socket
  kOpenSocketFailed = -12,

  /// Could not bind a socket
  kBindSocketFailed = -13,

  /// Could not listen on socket
  kListenSocketFailed = -14,

  /// Could not set a socket option
  kSetSocketOptionFailed = -15,

  /// Invalid regular expression
  kInvalidRegex = -16,

  /// Could not open file
  kOpenFileFailed = -17,

  /// Could not read from or write to socket
  kRwSocketFailed = -18,

  /// Could not connect a socket
  kConnectSocketFailed = -19,

  /// The magic prefix sent when establishing a connection is wrong
  kInvalidMagicPrefix = -20,

  /// The local and remote branches use incompatible Yogi versions
  kIncompatibleVersion = -21,

  /// Could not deserialize a message
  kDeserializeMsgFailed = -22,

  /// Could not accept a socket
  kAcceptSocketFailed = -23,

  /// Attempting to connect branch to itself
  kLoopbackConnection = -24,

  /// The passwords of the local and remote branch don't match
  kPasswordMismatch = -25,

  /// The net names of the local and remote branch don't match
  kNetNameMismatch = -26,

  /// A branch with the same name is already active
  kDuplicateBranchName = -27,

  /// A branch with the same path is already active
  kDuplicateBranchPath = -28,

  /// Message is too large
  kMessageTooLarge = -29,

  /// Parsing the command line failed
  kParsingCmdlineFailed = -30,

  /// Parsing a JSON string failed
  kParsingJsonFailed = -31,

  /// Parsing a configuration file failed
  kParsingFileFailed = -32,

  /// The configuration is not valid
  kConfigNotValid = -33,

  /// Help/usage text requested
  kHelpRequested = -34,

  /// Could not write to file
  kWriteToFileFailed = -35,

  /// One or more configuration variables are undefined or could not be
  /// resolved
  kUndefinedVariables = -36,

  /// Support for configuration variables has been disabled
  kNoVariableSupport = -37,

  /// A configuration variable has been used in a key
  kVariableUsedInKey = -38,

  /// Invalid time format
  kInvalidTimeFormat = -39,

  /// Could not parse time string
  kParsingTimeFailed = -40
};

template <>
inline std::string ToString<ErrorCode>(const ErrorCode& ec) {
  switch (ec) {
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kOk)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kUnknown)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kObjectStillUsed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kBadAlloc)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kInvalidParam)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kInvalidHandle)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kWrongObjectType)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kCanceled)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kBusy)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kTimeout)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kTimerExpired)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kBufferTooSmall)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kOpenSocketFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kBindSocketFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kListenSocketFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kSetSocketOptionFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kInvalidRegex)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kOpenFileFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kRwSocketFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kConnectSocketFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kInvalidMagicPrefix)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kIncompatibleVersion)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kDeserializeMsgFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kAcceptSocketFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kLoopbackConnection)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kPasswordMismatch)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kNetNameMismatch)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kDuplicateBranchName)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kDuplicateBranchPath)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kMessageTooLarge)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kParsingCmdlineFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kParsingJsonFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kParsingFileFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kConfigNotValid)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kHelpRequested)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kWriteToFileFailed)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kUndefinedVariables)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kNoVariableSupport)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kVariableUsedInKey)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kInvalidTimeFormat)
    YOGI_TO_STRING_ENUM_CASE(ErrorCode, kParsingTimeFailed)
  }

  bool should_never_get_here = false;
  assert(should_never_get_here);
  return {};
}

////////////////////////////////////////////////////////////////////////////////
/// Represents a result of an operation.
///
/// This is a wrapper around the result code returned by the functions from
/// the Yogi Core library. A result is represented by a number which is >= 0
/// in case of success and < 0 in case of a failure.
////////////////////////////////////////////////////////////////////////////////
class Result {
 public:
  /// Constructor
  ///
  /// \param value Result code as returned by the Yogi Core library function.
  explicit Result(int value) : value_(value) {}

  /// Constructor
  ///
  /// \param ec Error code.
  explicit Result(ErrorCode ec) : Result(static_cast<int>(ec)) {}

  virtual ~Result() {}

  /// Returns the result code.
  ///
  /// \returns The number as returned by the Yogi Core library function.
  int GetValue() const { return value_; }

  /// Error code associated with this result.
  ///
  /// \returns Associated error code.
  ErrorCode GetErrorCode() const {
    return value_ >= 0 ? ErrorCode::kOk : static_cast<ErrorCode>(value_);
  }

  /// Returns a human-readable string describing the result.
  ///
  /// \return Description of the result.
  virtual std::string ToString() const {
    return internal::YOGI_GetErrorString(value_);
  }

  /// Returns _true_ if the error code >= 0 (i.e. the operation succeeded).
  ///
  /// \returns _true_ if the error code >= 0.
  explicit operator bool() const { return value_ >= 0; }

  /// Returns _true_ if the error code < 0 (i.e. the operation failed).
  ///
  /// \returns _true_ if the error code < 0.
  bool operator!() const { return value_ < 0; }

  bool operator==(const Result& rhs) const { return value_ == rhs.value_; }
  bool operator!=(const Result& rhs) const { return value_ != rhs.value_; }
  bool operator<(const Result& rhs) const { return value_ < rhs.value_; }
  bool operator<=(const Result& rhs) const { return value_ <= rhs.value_; }
  bool operator>(const Result& rhs) const { return value_ > rhs.value_; }
  bool operator>=(const Result& rhs) const { return value_ >= rhs.value_; }

 private:
  const int value_;
};

////////////////////////////////////////////////////////////////////////////////
/// Represents the failure of an operation.
///
/// The success of an operation is associated with a result code >= 0.
////////////////////////////////////////////////////////////////////////////////
class Success : public Result {
 public:
  /// Constructor
  ///
  /// \param value Result code as returned by the Yogi Core library function.
  explicit Success(int value) : Result(value) {
    assert(value >= 0);  // The result code for Success must be >= 0.
  }

  /// Default constructor
  Success() : Success(static_cast<int>(ErrorCode::kOk)) {}
};

////////////////////////////////////////////////////////////////////////////////
/// Represents the failure of an operation.
///
/// The failure of an operation is associated with a result code < 0.
////////////////////////////////////////////////////////////////////////////////
class Failure : public Result {
 public:
  /// Constructor
  ///
  /// \param ec Error code.
  explicit Failure(ErrorCode ec) : Result(ec) {}
};

////////////////////////////////////////////////////////////////////////////////
/// A failure of an operation that includes a description.
///
/// Some functions in the Yogi Core library provide information in addition to
/// the error code in case of a failure. This class contains both the error
/// code and the additional information.
////////////////////////////////////////////////////////////////////////////////
class DescriptiveFailure : public Failure {
 public:
  /// Constructor
  ///
  /// \param ec          Error code.
  /// \param description Description of the error.
  DescriptiveFailure(ErrorCode ec, StringView description)
      : Failure(ec), description_(description) {}

  /// Returns a detailed description of the error.
  ///
  /// \returns Detailed error description.
  const std::string& GetDescription() const { return description_; }

  /// Returns a human-readable string describing the failure, including the
  /// additional error description.
  ///
  /// \return Detailed description of the failure.
  virtual std::string ToString() const override {
    return Failure::ToString() + ". Description: " + description_;
  }

 private:
  const std::string description_;
};

////////////////////////////////////////////////////////////////////////////////
/// Base class for all Yogi exceptions.
///
/// All exceptions thrown by Yogi functions are derived from this class.
///
/// \warning
///   Not thread safe!
////////////////////////////////////////////////////////////////////////////////
class Exception : public std::exception {
 public:
  /// Returns a description of the error.
  ///
  /// \returns Description of the error.
  virtual const char* what() const noexcept override = 0;
};

/// Exception wrapping a Failure object.
///
/// This exception type is used for failures without a detailed description.
class FailureException : public Exception {
 public:
  /// Constructor
  ///
  /// \param ec Error code.
  FailureException(ErrorCode ec) : failure_(ec) {}

  /// Returns the wrapped Failure object.
  ///
  /// \returns The wrapped Failure object.
  virtual const Failure& GetFailure() const { return failure_; }

  virtual const char* what() const noexcept override {
    if (what_.empty()) {
      what_ = GetFailure().ToString();
    }

    return what_.c_str();
  }

 private:
  const Failure failure_;
  mutable std::string what_;
};

////////////////////////////////////////////////////////////////////////////////
/// An exception class wrapping a DescriptiveFailure object.
///
/// This exception type is used for failures that have detailed information
/// available.
////////////////////////////////////////////////////////////////////////////////
class DescriptiveFailureException : public FailureException {
 public:
  /// Constructor
  ///
  /// \param ec          Error code.
  /// \param description Description of the error.
  DescriptiveFailureException(ErrorCode ec, StringView description)
      : FailureException(ec), failure_(ec, description) {}

  virtual const Failure& GetFailure() const override { return failure_; }

 private:
  const DescriptiveFailure failure_;
};

////////////////////////////////////////////////////////////////////////////////
/// An exception class for arithmetic errors.
///
/// This exception type is used when arithmetic errors occur in Yogi classes
/// such as Duration and Timestamp.
////////////////////////////////////////////////////////////////////////////////
class ArithmeticException : public Exception {
 public:
  /// Constructor
  ///
  /// \param what Description of the error.
  ArithmeticException(const char* what) : what_(what) {}

  virtual const char* what() const noexcept override { return what_; }

 private:
  const char* what_;
};

}  // namespace yogi

#endif  // YOGI_ERRORS_H

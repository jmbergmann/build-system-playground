#pragma once

#include "io.h"
#include "object.h"
#include "time.h"
#include "internal/string_conversion.h"
#include "internal/library.h"

#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <mutex>
#include <sstream>

/// Creates a log entry with fatal severity.
///
/// Examples:
/// \code
///  // Logging to the App logger:
///  YOGI_LOG_FATAL("My luck number is " << 42)
///
///  // Logging to a custom logger:
///  float rpm = 123.45;
///  yogi::Logger logger("Engine");
///  YOGI_LOG_FATAL(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_FATAL(...) YOGI_LOG(kFatal, __VA_ARGS__)

/// Creates a log entry with error severity.
///
/// Examples:
/// \code
///  // Logging to the App logger:
///  YOGI_LOG_ERROR("My luck number is " << 42)
///
///  // Logging to a custom logger:
///  float rpm = 123.45;
///  yogi::Logger logger("Engine");
///  YOGI_LOG_ERROR(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_ERROR(...) YOGI_LOG(kError, __VA_ARGS__)

/// Creates a log entry with warning severity.
///
/// Examples:
/// \code
///  // Logging to the App logger:
///  YOGI_LOG_WARNING("My luck number is " << 42)
///
///  // Logging to a custom logger:
///  float rpm = 123.45;
///  yogi::Logger logger("Engine");
///  YOGI_LOG_WARNING(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_WARNING(...) YOGI_LOG(kWarning, __VA_ARGS__)

/// Creates a log entry with info severity.
///
/// Examples:
/// \code
///  // Logging to the App logger:
///  YOGI_LOG_INFO("My luck number is " << 42)
///
///  // Logging to a custom logger:
///  float rpm = 123.45;
///  yogi::Logger logger("Engine");
///  YOGI_LOG_INFO(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_INFO(...) YOGI_LOG(kInfo, __VA_ARGS__)

/// Creates a log entry with debug severity.
///
/// Examples:
/// \code
///  // Logging to the App logger:
///  YOGI_LOG_DEBUG("My luck number is " << 42)
///
///  // Logging to a custom logger:
///  float rpm = 123.45;
///  yogi::Logger logger("Engine");
///  YOGI_LOG_DEBUG(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_DEBUG(...) YOGI_LOG(kDebug, __VA_ARGS__)

/// Creates a log entry with trace severity.
///
/// Examples:
/// \code
///  // Logging to the App logger:
///  YOGI_LOG_TRACE("My luck number is " << 42)
///
///  // Logging to a custom logger:
///  float rpm = 123.45;
///  yogi::Logger logger("Engine");
///  YOGI_LOG_TRACE(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_TRACE(...) YOGI_LOG(kTrace, __VA_ARGS__)

/// Creates a log entry.
///
/// The severity parameter is the name of one of the Verbosity enum values.
///
/// Examples:
/// \code
///  // Logging to the App logger:
///  YOGI_LOG(kInfo, "My luck number is " << 42)
///
///  // Logging to a custom logger:
///  float rpm = 123.45;
///  yogi::Logger logger("Engine");
///  YOGI_LOG(kWarning, logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG(severity, ...)                                               \
  _YOGI_LOG_EXPAND(_YOGI_LOG_IMPL(                                            \
      severity, _YOGI_LOG_SELECT(_YOGI_LOG_LOGGER, __VA_ARGS__)(__VA_ARGS__), \
      _YOGI_LOG_SELECT(_YOGI_LOG_STREAM, __VA_ARGS__)(__VA_ARGS__)))

#define _YOGI_LOG_EXPAND(x) x
#define _YOGI_LOG_CAT(a, b) _YOGI_LOG_CAT2(a, b)
#define _YOGI_LOG_CAT2(a, b) a##b

#define _YOGI_LOG_SELECT(macro, ...)                                  \
  _YOGI_LOG_CAT(macro, _YOGI_LOG_SELECT2(__VA_ARGS__, _CUSTOM_LOGGER, \
                                         _APP_LOGGER, _INVALID))

#define _YOGI_LOG_SELECT2(_0, _1, suffix, ...) suffix
#define _YOGI_LOG_LOGGER_CUSTOM_LOGGER(logger, stream) logger
#define _YOGI_LOG_LOGGER_APP_LOGGER(stream) ::yogi::app_logger
#define _YOGI_LOG_STREAM_CUSTOM_LOGGER(logger, stream) stream
#define _YOGI_LOG_STREAM_APP_LOGGER(stream) stream

#define _YOGI_LOG_IMPL(severity, logger, stream)                               \
  {                                                                            \
    if (::yogi::Verbosity::severity <= (logger).GetVerbosity()) {              \
      std::stringstream ss;                                                    \
      ss << stream;                                                            \
      (logger).Log(::yogi::Verbosity::severity, ss.str(), __FILE__, __LINE__); \
    }                                                                          \
  }

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_LogToConsole,
                   (int verbosity, int stream, int color, const char* timefmt,
                    const char* fmt))
YOGI_DEFINE_API_FN(int, YOGI_LogToHook,
                   (int verbosity,
                    void (*fn)(int severity, long long timestamp, int tid,
                               const char* file, int line, const char* comp,
                               const char* msg, void* userarg),
                    void* userarg))
YOGI_DEFINE_API_FN(int, YOGI_LogToFile,
                   (int verbosity, const char* filename, char* genfn,
                    int genfnsize, const char* timefmt, const char* fmt))
YOGI_DEFINE_API_FN(int, YOGI_LoggerCreate,
                   (void** logger, const char* component))
YOGI_DEFINE_API_FN(int, YOGI_LoggerGetVerbosity, (void* logger, int* verbosity))
YOGI_DEFINE_API_FN(int, YOGI_LoggerSetVerbosity, (void* logger, int verbosity))
YOGI_DEFINE_API_FN(int, YOGI_LoggerSetComponentsVerbosity,
                   (const char* components, int verbosity, int* count))
YOGI_DEFINE_API_FN(int, YOGI_LoggerLog,
                   (void* logger, int severity, const char* file, int line,
                    const char* msg))

/// Levels of how verbose logging output is.
///
/// The term "severity" is refers to the same type.
enum class Verbosity {
  /// Fatal errors are errors that require a process restart.
  kFatal = 0,

  /// Errors that the system can recover from.
  kError = 1,

  /// >Warnings.
  kWarning = 2,

  /// Useful general information about the system state.
  kInfo = 3,

  /// Information for debugging.
  kDebug = 4,

  /// Detailed debugging information.
  kTrace = 5,
};

template <>
inline std::string ToString<Verbosity>(const Verbosity& vb) {
  switch (vb) {
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kFatal)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kError)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kWarning)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kInfo)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kDebug)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kTrace)
  }

  bool should_never_get_here = false;
  assert(should_never_get_here);
  return {};
}

/// Output streams for writing to the terminal.
enum class Stream {
  /// Standard output.
  kStdout = 0,

  /// Stanard error.
  kStderr = 1,
};

template <>
inline std::string ToString<Stream>(const Stream& st) {
  switch (st) {
    YOGI_TO_STRING_ENUM_CASE(Stream, kStdout)
    YOGI_TO_STRING_ENUM_CASE(Stream, kStderr)
  }

  bool should_never_get_here = false;
  assert(should_never_get_here);
  return {};
}

/// Allows the YOGI to write library-internal and user logging to stdout or
/// stderr.
///
/// This function supports colourizing the output if the terminal that the
/// process is running in supports it. The color used for a log entry depends
/// on the entry's severity. For example, errors will be printed in red and
/// warnings in yellow.
///
/// Each log entry contains the *component* tag which describes which part of
/// a program issued the log entry. For entries created by the library itself,
/// this parameter is prefixed with the string "Yogi.", followed by the
/// internal component name. For example, the component tag for a branch would
/// be "Yogi.Branch".
///
/// The \p timefmt parameter describes the textual format of a log entry's
/// timestamp. The following placeholders are supported:
///     %Y: Four digit year.
///     %m: Month name as a decimal 01 to 12.
///     %d: Day of the month as decimal 01 to 31.
///     %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
///     %H: The hour as a decimal number using a 24-hour clock (00 to 23).
///     %M: The minute as a decimal 00 to 59.
///     %S: Seconds as a decimal 00 to 59.
///     %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
///     %3: Milliseconds as decimal number 000 to 999.
///     %6: Microseconds as decimal number 000 to 999.
///     %9: Nanoseconds as decimal number 000 to 999.
///
/// The \p fmt parameter describes the textual format of the complete log entry
/// as it will appear on the console. The supported placeholders are:
///     $t: Timestamp, formatted according to the timefmt parameter.
///     $P: Process ID (PID).
///     $T: Thread ID.
///     $s: Severity as 3 letters (FAT, ERR, WRN, IFO, DBG or TRC).
///     $m: Log message.
///     $f: Source file name.
///     $l: Source line number.
///     $c: Component tag.
///     $<: Set console color corresponding to severity.
///     $>: Reset the colours (also done after each log entry).
///     $$: A $ sign.
inline void LogToConsole(Verbosity verbosity, Stream stream = Stream::kStderr,
                         bool color = true, const std::string& timefmt = {},
                         const std::string& fmt = {}) {
  int res = internal::YOGI_LogToConsole(
      static_cast<int>(verbosity), static_cast<int>(stream), color ? 1 : 0,
      timefmt.empty() ? nullptr : timefmt.c_str(),
      fmt.empty() ? nullptr : fmt.c_str());
  internal::CheckErrorCode(res);
}

/// Disables logging to the console.
inline void LogToConsole() {
  int res = internal::YOGI_LogToConsole(-1, 0, 0, nullptr, nullptr);
  internal::CheckErrorCode(res);
}

/// Handler function type for the log hook.
///
/// The parameters are:
///  -# *severity*: Severity (verbosity) of the entry.
///  -# *timestamp*: UTC Timestamp of the entry.
///  -# *tid*: ID of the thread that created the entry.
///  -# *file*: Source file name.
///  -# *line*: Source file line number.
///  -# *comp*: Component that created the entry.
///  -# *msg*: Log message.
typedef std::function<void(Verbosity severity, Timestamp timestamp, int tid,
                           std::string file, int line, std::string comp,
                           std::string msg)>
    LogHookFn;

namespace internal {

struct LogToHookData {
  static std::mutex mutex;
  static std::unique_ptr<LogHookFn> log_hook_fn;
};

YOGI_WEAK_SYMBOL std::mutex LogToHookData::mutex;
YOGI_WEAK_SYMBOL std::unique_ptr<LogHookFn> LogToHookData::log_hook_fn;

}  // namespace internal

/// Installs a callback function for receiving log entries.
///
/// This function can be used to get notified whenever the Yogi library itself
/// or the user produces log messages. These messages can then be processed
/// further in user code.
///
/// \param verbosity Maximum verbosity of messages to log.
/// \param fn Callback function.
inline void LogToHook(Verbosity verbosity, LogHookFn fn) {
  auto fn_ptr = std::make_unique<LogHookFn>(fn);
  static auto wrapper = [](int severity, long long timestamp, int tid,
                           const char* file, int line, const char* comp,
                           const char* msg, void* userarg) {
    auto fn_ptr = (static_cast<LogHookFn*>(userarg));
    (*fn_ptr)(
        static_cast<Verbosity>(severity),
        Timestamp::FromDurationSinceEpoch(Duration::FromNanoseconds(timestamp)),
        tid, file ? file : "", line, comp, msg ? msg : "");
  };
  std::lock_guard<std::mutex> lock(internal::LogToHookData::mutex);

  int res = internal::YOGI_LogToHook(static_cast<int>(verbosity), wrapper,
                                     fn_ptr.get());
  internal::CheckErrorCode(res);

  internal::LogToHookData::log_hook_fn = std::move(fn_ptr);
}

/// Disables logging to the hook function.
inline void LogToHook() {
  std::lock_guard<std::mutex> lock(internal::LogToHookData::mutex);

  int res = internal::YOGI_LogToHook(-1, nullptr, nullptr);
  internal::CheckErrorCode(res);

  internal::LogToHookData::log_hook_fn = {};
}

/// Creates a log file.
///
/// This function opens a file to write library-internal and user logging
/// information to. If the file with the given filename already exists then it
/// will be overwritten.
///
/// The filename parameter supports all placeholders that are valid for the
/// timefmt parameter (see below).
///
/// The \p timefmt parameter describes the textual format of a log entry's
/// timestamp. The following placeholders are supported:
///     %Y: Four digit year.
///     %m: Month name as a decimal 01 to 12.
///     %d: Day of the month as decimal 01 to 31.
///     %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
///     %H: The hour as a decimal number using a 24-hour clock (00 to 23).
///     %M: The minute as a decimal 00 to 59.
///     %S: Seconds as a decimal 00 to 59.
///     %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
///     %3: Milliseconds as decimal number 000 to 999.
///     %6: Microseconds as decimal number 000 to 999.
///     %9: Nanoseconds as decimal number 000 to 999.
///
/// The \p fmt parameter describes the textual format of the complete log entry
/// as it will in the log file. The supported placeholders are:
///     $t: Timestamp, formatted according to the timefmt parameter.
///     $P: Process ID (PID).
///     $T: Thread ID.
///     $s: Severity as 3 letters (FAT, ERR, WRN, IFO, DBG or TRC).
///     $m: Log message.
///     $f: Source file name.
///     $l: Source line number.
///     $c: Component tag.
///     $$: A $ sign.
///
/// \param verbosity Maximum verbosity of messages to log.
/// \param filename Path to the log file (see above for placeholders).
/// \param timefmt Format of the timestamp (see above for placeholders).
/// \param fmt Format of a log entry (see above for placeholders).
///
/// \returns The generated filename with all placeholders resolved.
inline std::string LogToFile(Verbosity verbosity, const std::string& filename,
                             const std::string& timefmt = {},
                             const std::string& fmt = {}) {
  char genfn[256];
  int res = internal::YOGI_LogToFile(
      static_cast<int>(verbosity), filename.c_str(), genfn, sizeof(genfn),
      timefmt.empty() ? nullptr : timefmt.c_str(),
      fmt.empty() ? nullptr : timefmt.c_str());
  internal::CheckErrorCode(res);
  return genfn;
}

/// Disables logging to a file
inline void LogToFile() {
  int res = internal::YOGI_LogToFile(-1, nullptr, nullptr, 0, nullptr, nullptr);
  internal::CheckErrorCode(res);
}

/// Allows generating log entries.
///
/// A logger is an object used for generating log entries that are tagged with
/// the logger's component tag. A logger's component tag does not have to be
/// unique, i.e. multiple loggers can be created using identical component
/// tags.
///
/// The verbosity of a logger acts as a filter. Only messages with a verbosity
/// less than or equal to the logger's verbosity are being logged.
///
/// Note: The verbosity of a logger affects only messages logged through that
///       particular logger, i.e. if two loggers have identical component tags
///       their verbosity settings are still independent from each other.
class Logger : public Object {
 public:
  /// Sets the verbosity of all loggers matching a given component tag.
  ///
  /// This function finds all loggers whose component tag matches the regular
  /// expression given in the components parameter and sets their verbosity
  /// to the value of the verbosity parameter.
  ///
  /// \param components Regex (ECMAScript) for the component tags to match.
  /// \param verbosity Maximum verbosity entries to be logged.
  ///
  /// \returns Number of matching loggers.
  static int SetComponentsVerbosity(const std::string& components,
                                    Verbosity verbosity) {
    int count;
    int res = internal::YOGI_LoggerSetComponentsVerbosity(
        components.c_str(), static_cast<int>(verbosity), &count);
    internal::CheckErrorCode(res);
    return count;
  }

  /// Constructor.
  ///
  /// The verbosity of new loggers is Verbosity.Info by default.
  ///
  /// \param component The component tag to use.
  Logger(const std::string& component)
      : Object(internal::CallApiCreate(internal::YOGI_LoggerCreate,
                                       component.c_str())) {}

  /// For the AppLogger
  Logger() : Object(nullptr) {}

  /// Returns the verbosity of the logger.
  ///
  /// \returns Verbosity of the logger.
  Verbosity GetVerbosity() const {
    int vb;
    int res = internal::YOGI_LoggerGetVerbosity(GetHandle(), &vb);
    internal::CheckErrorCode(res);
    return static_cast<Verbosity>(vb);
  }

  /// Sets the verbosity of the logger.
  ///
  /// \param verbosity Verbosity.
  void SetVerbosity(Verbosity verbosity) {
    int res = internal::YOGI_LoggerSetVerbosity(GetHandle(),
                                                static_cast<int>(verbosity));
    internal::CheckErrorCode(res);
  }

  /// Creates a log entry.
  ///
  /// \param severity Severity (verbosity) of the entry.
  /// \param msg Log message.
  /// \param file Source file name.
  /// \param line Source file line number.
  template <typename MsgString, typename FileString>
  void Log(Verbosity severity, const MsgString& msg, const FileString& file,
           int line) {
    const char* short_file = internal::StringToCoreString(file);
    if (short_file) {
      for (const char* ch = short_file; *ch; ++ch) {
        if (*ch == '/' || *ch == '\\') {
          short_file = ch + 1;
        }
      }
    }

    int res = internal::YOGI_LoggerLog(GetHandle(), static_cast<int>(severity),
                                       short_file, line,
                                       internal::StringToCoreString(msg));
    internal::CheckErrorCode(res);
  }

  /// Creates a log entry.
  ///
  /// \param severity Severity (verbosity) of the entry.
  /// \param msg Log message.
  template <typename MsgString>
  void Log(Verbosity severity, const MsgString& msg) {
    return Log<MsgString, const char*>(severity, msg, nullptr, 0);
  }
};

/// Represents the App logger singleton.
///
/// The App logger always exists and uses "App" as its component tag.
/// Instances of this class can be created; however, they will always point to
/// the same logger, i.e. changing its verbosity will change the verbosity of
/// every AppLogger instance.
class AppLogger : public Logger {
 public:
  virtual std::string ToString() const override { return "AppLogger"; }
};

static AppLogger app_logger;

}  // namespace yogi

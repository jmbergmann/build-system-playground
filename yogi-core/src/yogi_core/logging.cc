#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../objects/logger.h"

#include <regex>

YOGI_API int YOGI_LogToConsole(int verbosity, int stream, int colour,
                               const char* timefmt, const char* fmt) {
  if (verbosity != YOGI_VB_NONE) {
    CHECK_PARAM(YOGI_VB_FATAL <= verbosity && verbosity <= YOGI_VB_TRACE);
    CHECK_PARAM(stream == YOGI_ST_STDOUT || stream == YOGI_ST_STDERR);
    CHECK_PARAM(colour == YOGI_TRUE || colour == YOGI_FALSE);
    CHECK_PARAM(timefmt == nullptr || IsTimeFormatValid(timefmt));
    CHECK_PARAM(fmt == nullptr || IsLogFormatValid(fmt));
  }

  try {
    if (verbosity == YOGI_VB_NONE) {
      objects::Logger::SetSink(objects::detail::ConsoleLogSinkPtr());
    } else {
      objects::Logger::SetSink(
          std::make_unique<objects::detail::ConsoleLogSink>(
              static_cast<objects::Logger::Verbosity>(verbosity),
              stream == YOGI_ST_STDOUT ? stdout : stderr, !!colour,
              timefmt ? timefmt : api::kDefaultLogTimeFormat,
              fmt ? fmt : api::kDefaultLogFormat));
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_LogToHook(int verbosity,
                            void (*fn)(int, long long, int, const char*, int,
                                       const char*, const char*, void*),
                            void* userarg) {
  CHECK_PARAM(YOGI_VB_NONE <= verbosity && verbosity <= YOGI_VB_TRACE);

  try {
    if (fn == nullptr || verbosity == YOGI_VB_NONE) {
      objects::Logger::SetSink(objects::detail::HookLogSinkPtr());
    } else {
      auto hook_fn = [fn, userarg](auto severity, auto& time, int tid,
                                   auto file, int line, auto& component,
                                   auto msg) {
        fn(severity, time.NanosecondsSinceEpoch().count(), tid, file, line,
           component.c_str(), msg, userarg);
      };
      objects::Logger::SetSink(std::make_unique<objects::detail::HookLogSink>(
          static_cast<objects::Logger::Verbosity>(verbosity), hook_fn));
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_LogToFile(int verbosity, const char* filename, char* genfn,
                            int genfnsize, const char* timefmt,
                            const char* fmt) {
  if (verbosity != YOGI_VB_NONE) {
    CHECK_PARAM(YOGI_VB_FATAL <= verbosity && verbosity <= YOGI_VB_TRACE);
    CHECK_PARAM(filename == nullptr || IsTimeFormatValid(filename));
    CHECK_PARAM(genfn == nullptr || genfnsize > 0);
    CHECK_PARAM(timefmt == nullptr || IsTimeFormatValid(timefmt));
    CHECK_PARAM(fmt == nullptr || IsLogFormatValid(fmt));
  }

  try {
    // Remove existing sink first in order to close the old log file
    objects::Logger::SetSink(objects::detail::FileLogSinkPtr());
    if (filename != nullptr && verbosity != YOGI_VB_NONE) {
      auto sink = std::make_unique<objects::detail::FileLogSink>(
          static_cast<objects::Logger::Verbosity>(verbosity), filename,
          timefmt ? timefmt : api::kDefaultLogTimeFormat,
          fmt ? fmt : api::kDefaultLogFormat);
      auto gen_filename = sink->GetGeneratedFilename();
      objects::Logger::SetSink(std::move(sink));

      if (genfn) {
        auto n =
            std::min(gen_filename.size() + 1, static_cast<size_t>(genfnsize));
        strncpy(genfn, gen_filename.c_str(), n);
        if (gen_filename.size() + 1 > n) {
          genfn[genfnsize - 1] = '\0';
          return YOGI_ERR_BUFFER_TOO_SMALL;
        }
      }
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_LoggerCreate(void** logger, const char* component) {
  CHECK_PARAM(logger != nullptr);
  CHECK_PARAM(component != nullptr && *component != '\0');

  try {
    auto log = objects::Logger::Create(component);
    *logger = api::ObjectRegister::Register(log);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_LoggerSetVerbosity(void* logger, int verbosity) {
  CHECK_PARAM(YOGI_VB_NONE <= verbosity && verbosity <= YOGI_VB_TRACE);

  try {
    auto log = logger ? api::ObjectRegister::Get<objects::Logger>(logger)
                      : objects::Logger::GetAppLogger();
    log->SetVerbosity(static_cast<objects::Logger::Verbosity>(verbosity));
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_LoggerGetVerbosity(void* logger, int* verbosity) {
  CHECK_PARAM(verbosity != nullptr);

  try {
    auto log = logger ? api::ObjectRegister::Get<objects::Logger>(logger)
                      : objects::Logger::GetAppLogger();
    *verbosity = log->GetVerbosity();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_LoggerSetComponentsVerbosity(const char* components,
                                               int verbosity, int* count) {
  CHECK_PARAM(components != nullptr && *components != '\0');
  CHECK_PARAM(YOGI_VB_NONE <= verbosity && verbosity <= YOGI_VB_TRACE);

  try {
    std::regex re(components);
    std::smatch m;
    int n = 0;

    auto fn = [&](const objects::LoggerPtr& log) {
      if (std::regex_match(log->GetComponent(), m, re)) {
        log->SetVerbosity(static_cast<objects::Logger::Verbosity>(verbosity));
        ++n;
      }
    };

    // App logger
    fn(objects::Logger::GetAppLogger());

    // Loggers created by the user
    for (auto& log : api::ObjectRegister::GetAll<objects::Logger>()) {
      fn(log);
    }

    // Internal loggers
    for (auto& weak_log : objects::Logger::GetInternalLoggers()) {
      auto log = weak_log.lock();
      if (log) {
        fn(log);
      }
    }

    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_LoggerLog(void* logger, int severity, const char* file,
                            int line, const char* msg) {
  CHECK_PARAM(YOGI_VB_NONE <= severity && severity <= YOGI_VB_TRACE);
  CHECK_PARAM(file == nullptr || *file != '\0');
  CHECK_PARAM(file == nullptr || line >= 0);
  CHECK_PARAM(msg != nullptr && *msg != '\0');

  try {
    auto log = logger ? api::ObjectRegister::Get<objects::Logger>(logger)
                      : objects::Logger::GetAppLogger();
    log->Log(static_cast<objects::Logger::Verbosity>(severity), file, line,
             msg);
  }
  CATCH_AND_RETURN;
}
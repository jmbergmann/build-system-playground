#include "config.h"
#include "../include/yogi_core.h"
#include "api/constants.h"
#include "api/error.h"
#include "licenses/licenses.h"
#include "objects/context.h"
#include "objects/branch.h"
#include "objects/logger.h"
#include "objects/timer.h"
#include "utils/system.h"

#include <boost/algorithm/string.hpp>
#include <stdexcept>
#include <regex>

#define CHECK_PARAM(cond)                       \
  {                                             \
    if (!(cond)) return YOGI_ERR_INVALID_PARAM; \
  }

#define CHECK_BUFFER_PARAMS(buffer, size)        \
  {                                              \
    CHECK_PARAM(buffer != nullptr || size == 0); \
    CHECK_PARAM(size >= 0);                      \
  }

#ifdef NDEBUG
#define CATCH_AND_RETURN_INTERNAL_ERRORS                               \
  catch (const std::exception& e) {                                    \
    fprintf(stderr, "%s:%i: INTERNAL ERROR: %s\n", __FILE__, __LINE__, \
            e.what());                                                 \
    return YOGI_ERR_UNKNOWN;                                           \
  }                                                                    \
  catch (...) {                                                        \
    fprintf(stderr, "%s:%i: INTERNAL ERROR: %s\n", __FILE__, __LINE__, \
            "Unknown error");                                          \
    return YOGI_ERR_UNKNOWN;                                           \
  }
#else
#define CATCH_AND_RETURN_INTERNAL_ERRORS
#endif

#define CATCH_AND_RETURN            \
  catch (const api::Error& err) {   \
    return err.error_code();        \
  }                                 \
  catch (const std::bad_alloc&) {   \
    return YOGI_ERR_BAD_ALLOC;      \
  }                                 \
  catch (const std::regex_error&) { \
    return YOGI_ERR_INVALID_REGEX;  \
  }                                 \
  CATCH_AND_RETURN_INTERNAL_ERRORS  \
  return YOGI_OK;

namespace {

std::chrono::nanoseconds ConvertDuration(long long duration) {
  if (duration == -1) {
    return std::chrono::nanoseconds::max();
  } else {
    return std::chrono::nanoseconds(duration);
  }
}

objects::Branch::BranchEvents ConvertBranchEvents(int events) {
  return events ? static_cast<objects::Branch::BranchEvents>(events)
                : objects::Branch::BranchEvents::kAllEvents;
}

bool IsTimeFormatValid(const std::string& fmt) {
  if (fmt.empty()) return false;

  std::regex re("%([^YmdFHMST369]|$)");
  return !std::regex_search(fmt, re);
}

bool IsLogFormatValid(std::string fmt) {
  if (fmt.empty()) return false;

  boost::replace_all(fmt, "$$", "");
  std::regex re("\\$([^tPTsmflc<>]|$)");
  return !std::regex_search(fmt, re);
}

void CopyUuidToUserBuffer(const boost::uuids::uuid& uuid, void* buffer) {
  if (buffer == nullptr) return;
  std::memcpy(buffer, &uuid, uuid.size());
}

bool CopyStringToUserBuffer(const std::string& str, char* buffer,
                            int buffer_size) {
  if (buffer == nullptr) return true;

  auto n = std::min(str.size() + 1, static_cast<std::size_t>(buffer_size));
  std::strncpy(buffer, str.c_str(), n);
  if (str.size() + 1 > n) {
    buffer[buffer_size - 1] = '\0';
    return false;
  }

  return true;
}

}  // anonymous namespace

YOGI_API const char* YOGI_GetVersion() { return api::kVersionNumber; }

YOGI_API const char* YOGI_GetLicense() {
  return licenses::kYogiLicense.c_str();
}

YOGI_API const char* YOGI_Get3rdPartyLicenses() {
  return licenses::k3rdPartyLicenses.c_str();
}

YOGI_API const char* YOGI_GetErrorString(int err) {
  return api::Error(err).what();
}

YOGI_API int YOGI_GetConstant(void* dest, int constant) {
  CHECK_PARAM(dest != nullptr);

  try {
    api::GetConstant(dest, constant);
  }
  CATCH_AND_RETURN;
}

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

YOGI_API int YOGI_Destroy(void* object) {
  CHECK_PARAM(object != nullptr);

  try {
    api::ObjectRegister::Destroy(object);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_DestroyAll() {
  try {
    for (auto& ctx : api::ObjectRegister::GetAll<objects::Context>()) {
      ctx->Stop();
      ctx->WaitForStopped(std::chrono::nanoseconds::max());
    }

    api::ObjectRegister::DestroyAll();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextCreate(void** context) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = objects::Context::Create();
    *context = api::ObjectRegister::Register(ctx);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextPoll(void* context, int* count) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    int n = ctx->Poll();
    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextPollOne(void* context, int* count) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    int n = ctx->PollOne();
    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextRun(void* context, int* count, long long duration) {
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(duration >= -1);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    int n = ctx->Run(ConvertDuration(duration));
    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextRunOne(void* context, int* count, long long duration) {
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(duration >= -1);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    int n = ctx->RunOne(ConvertDuration(duration));
    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextRunInBackground(void* context) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    ctx->RunInBackground();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextStop(void* context) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    ctx->Stop();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextWaitForRunning(void* context, long long duration) {
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(duration >= -1);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    if (!ctx->WaitForRunning(ConvertDuration(duration))) {
      return YOGI_ERR_TIMEOUT;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextWaitForStopped(void* context, long long duration) {
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(duration >= -1);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    if (!ctx->WaitForStopped(ConvertDuration(duration))) {
      return YOGI_ERR_TIMEOUT;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextPost(void* context, void (*fn)(void*), void* userarg) {
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(fn != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    ctx->Post([=] { fn(userarg); });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_TimerCreate(void** timer, void* context) {
  CHECK_PARAM(timer != nullptr);
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto tmr = objects::Timer::Create(ctx);
    *timer = api::ObjectRegister::Register(tmr);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_TimerStart(void* timer, long long duration,
                             void (*fn)(int, void*), void* userarg) {
  CHECK_PARAM(timer != nullptr);
  CHECK_PARAM(duration >= -1);
  CHECK_PARAM(fn != nullptr);

  try {
    auto tmr = api::ObjectRegister::Get<objects::Timer>(timer);
    auto timeout = ConvertDuration(duration);
    tmr->Start(timeout, [=](int res) { fn(res, userarg); });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_TimerCancel(void* timer) {
  CHECK_PARAM(timer != nullptr);

  try {
    auto tmr = api::ObjectRegister::Get<objects::Timer>(timer);
    if (!tmr->Cancel()) {
      return YOGI_ERR_TIMER_EXPIRED;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchCreate(void** branch, void* context, const char* name,
                               const char* description, const char* netname,
                               const char* password, const char* path,
                               const char* advaddr, int advport,
                               long long advint, long long timeout) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(name == nullptr || *name != '\0');
  CHECK_PARAM(netname == nullptr || *netname != '\0');
  CHECK_PARAM(password == nullptr || *password != '\0');
  CHECK_PARAM(path == nullptr || *path == '/');
  CHECK_PARAM(advaddr == nullptr || *advaddr != '\0');
  CHECK_PARAM(advport >= 0);
  CHECK_PARAM(advint == -1 || advint == 0 || advint >= 1000000);
  CHECK_PARAM(timeout == -1 || timeout == 0 || timeout >= 1000000);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto final_name = name ? std::string(name)
                           : std::to_string(utils::GetProcessId()) + '@' +
                                 utils::GetHostname();

    boost::system::error_code ec;
    auto adv_ep = boost::asio::ip::udp::endpoint(
        boost::asio::ip::make_address(
            advaddr ? advaddr : api::kDefaultAdvAddress, ec),
        static_cast<unsigned short>(advport ? advport : api::kDefaultAdvPort));
    if (ec) return YOGI_ERR_INVALID_PARAM;

    auto brn = objects::Branch::Create(
        ctx, final_name, description ? description : "",
        netname ? std::string(netname) : utils::GetHostname(),
        password ? password : "",
        path ? std::string(path) : (std::string("/") + final_name), adv_ep,
        advint ? ConvertDuration(advint)
               : std::chrono::nanoseconds(api::kDefaultAdvInterval),
        timeout ? ConvertDuration(timeout)
                : std::chrono::nanoseconds(api::kDefaultConnectionTimeout));
    brn->Start();

    *branch = api::ObjectRegister::Register(brn);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchGetInfo(void* branch, void* uuid, char* json,
                                int jsonsize) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);
    CopyUuidToUserBuffer(brn->GetUuid(), uuid);

    if (!CopyStringToUserBuffer(brn->MakeInfoString(), json, jsonsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchGetConnectedBranches(void* branch, void* uuid,
                                             char* json, int jsonsize,
                                             void (*fn)(int, void*),
                                             void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);

    auto buffer_too_small = false;
    for (auto& entry : brn->MakeConnectedBranchesInfoStrings()) {
      CopyUuidToUserBuffer(entry.first, uuid);

      if (CopyStringToUserBuffer(entry.second, json, jsonsize)) {
        fn(YOGI_OK, userarg);
      } else {
        fn(YOGI_ERR_BUFFER_TOO_SMALL, userarg);
        buffer_too_small = true;
      }
    }

    if (buffer_too_small) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchAwaitEvent(void* branch, int events, void* uuid,
                                   char* json, int jsonsize,
                                   void (*fn)(int, int, int, void*),
                                   void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);

    brn->AwaitEvent(
        ConvertBranchEvents(events), [=](auto& res, auto event, auto& evres,
                                         auto& tmp_uuid, auto& tmp_json) {
          if (res != api::kSuccess) {
            fn(res.error_code(), event, evres.error_code(), userarg);
            return;
          }

          CopyUuidToUserBuffer(tmp_uuid, uuid);
          if (CopyStringToUserBuffer(tmp_json, json, jsonsize)) {
            fn(res.error_code(), event, evres.error_code(), userarg);
          } else {
            fn(YOGI_ERR_BUFFER_TOO_SMALL, event, evres.error_code(), userarg);
          }
        });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchCancelAwaitEvent(void* branch) {
  CHECK_PARAM(branch != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);
    brn->CancelAwaitEvent();
  }
  CATCH_AND_RETURN;
}

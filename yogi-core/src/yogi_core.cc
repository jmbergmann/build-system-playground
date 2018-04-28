#include "config.h"
#include "../include/yogi_core.h"
#include "api/constants.h"
#include "api/error.h"
#include "objects/context.h"
#include "objects/branch.h"
#include "objects/timer.h"
#include "utils/system.h"

#include <stdexcept>

#define CHECK_PARAM(cond)                       \
  {                                             \
    if (!(cond)) return YOGI_ERR_INVALID_PARAM; \
  }

#define CHECK_BUFFER_PARAMS(buffer, size)        \
  {                                              \
    CHECK_PARAM(buffer != nullptr || size == 0); \
    CHECK_PARAM(size >= 0);                      \
  }

#define CHECK_TIMEOUT_PARAMS(seconds, nanoseconds)               \
  {                                                              \
    CHECK_PARAM(seconds >= -1);                                  \
    CHECK_PARAM(seconds == -1 ||                                 \
                (1000000000 > nanoseconds && nanoseconds >= 0)); \
  }

#define CATCH_AND_RETURN                                                  \
  catch (const api::Error& err) {                                         \
    return err.error_code();                                              \
  }                                                                       \
  catch (const std::bad_alloc&) {                                         \
    return YOGI_ERR_BAD_ALLOC;                                            \
  }                                                                       \
                                                                  \
  return YOGI_OK;

namespace {

std::chrono::nanoseconds ConvertTimeout(int seconds, int nanoseconds) {
  if (seconds == -1) {
    return std::chrono::nanoseconds::max();
  }
  else {
    return std::chrono::nanoseconds(nanoseconds) +
           std::chrono::seconds(seconds);
  }
}

}  // anonymous namespace

YOGI_API const char* YOGI_GetVersion() { return api::kVersionNumber; }

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

YOGI_API int YOGI_ContextRun(void* context, int* count) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    int n = ctx->Run();
    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextRunOne(void* context, int* count) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    int n = ctx->RunOne();
    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextRunFor(void* context, int* count, int seconds,
                                int nanoseconds) {
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(seconds >= 0);
  CHECK_PARAM(1000000000 > nanoseconds && nanoseconds >= 0);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto dur = std::chrono::nanoseconds(nanoseconds) + std::chrono::seconds(seconds);

    int n = ctx->RunFor(dur);
    if (count) {
      *count = n;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextRunOneFor(void* context, int* count, int seconds,
                                   int nanoseconds) {
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(seconds >= 0);
  CHECK_PARAM(1000000000 > nanoseconds && nanoseconds >= 0);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto dur = std::chrono::nanoseconds(nanoseconds) + std::chrono::seconds(seconds);

    int n = ctx->RunOneFor(dur);
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

YOGI_API int YOGI_ContextWaitForRunning(void* context, int seconds,
                                        int nanoseconds) {
  CHECK_PARAM(context != nullptr);
  CHECK_TIMEOUT_PARAMS(seconds, nanoseconds);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto timeout = ConvertTimeout(seconds, nanoseconds);

    if (!ctx->WaitForRunning(timeout)) {
      return YOGI_ERR_TIMEOUT;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ContextWaitForStopped(void* context, int seconds,
                                        int nanoseconds) {
  CHECK_PARAM(context != nullptr);
  CHECK_TIMEOUT_PARAMS(seconds, nanoseconds);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto timeout = ConvertTimeout(seconds, nanoseconds);

    if (!ctx->WaitForStopped(timeout)) {
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

YOGI_API int YOGI_TimerStart(void* timer, int seconds, int nanoseconds,
                             void (*fn)(int, void*), void* userarg) {
  CHECK_PARAM(timer != nullptr);
  CHECK_TIMEOUT_PARAMS(seconds, nanoseconds);
  CHECK_PARAM(fn != nullptr);

  try {
    auto tmr = api::ObjectRegister::Get<objects::Timer>(timer);
    auto timeout = ConvertTimeout(seconds, nanoseconds);
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
                               const char* advaddr, int advport, int advint) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(name == nullptr || *name != '\0');
  CHECK_PARAM(netname == nullptr || *netname != '\0');
  CHECK_PARAM(password == nullptr || *password != '\0');
  CHECK_PARAM(path == nullptr || *path == '/');
  CHECK_PARAM(advaddr == nullptr || *advaddr != '\0');
  CHECK_PARAM(advport >= 0);
  CHECK_PARAM(advint >= -1);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    auto final_name =
        name ? std::string(name)
             : std::to_string(utils::GetPid()) + '@' + utils::GetHostname();
    auto final_description = description ? description : "";
    auto final_netname = netname ? std::string(netname) : utils::GetHostname();
    auto final_password = password ? password : "";
    auto final_path =
        path ? std::string(path) : (std::string("/") + final_name);
    auto final_advaddr = advaddr ? advaddr : api::kDefaultAdvAddress;
    auto final_advport = advport ? advport : api::kDefaultAdvPort;
    auto final_advint = std::chrono::milliseconds::max();
    if (advint >= 0) {
      final_advint =
          std::chrono::milliseconds(advint ? advint : api::kDefaultAdvInterval);
    }

    auto brn = objects::Branch::Create(
        ctx, final_name, final_description, final_netname, final_password,
        final_path, final_advaddr, final_advport, final_advint);
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

    if (uuid) {
      auto& tmp = brn->GetUuid();
      std::memcpy(uuid, &tmp, tmp.size());
    }

    if (json) {
      auto tmp = brn->MakeInfo();
      auto n = std::min(tmp.size() + 1, static_cast<size_t>(jsonsize));
      strncpy(json, tmp.c_str(), n);
      if (tmp.size() + 1 > n) {
        json[jsonsize - 1] = '\0';
        return YOGI_ERR_BUFFER_TOO_SMALL;
      }
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchGetDiscoveredBranches(void* branch, void* uuid,
                                              char* json, int jsonsize,
                                              void (*fn)(int, void*),
                                              void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);

    auto buffer_too_small = false;
    if (json) {
      brn->ForeachDiscoveredBranch([&](auto& tmp_uuid, auto tmp_json) {
        if (uuid) {
          memcpy(uuid, &tmp_uuid, tmp_uuid.size());
        }

        auto n = std::min(tmp_json.size() + 1, static_cast<size_t>(jsonsize));
        std::strncpy(json, tmp_json.c_str(), n);
        if (tmp_json.size() + 1 > n) {
          json[jsonsize - 1] = '\0';
          fn(YOGI_ERR_BUFFER_TOO_SMALL, userarg);
          buffer_too_small = true;
        }
        else {
          fn(YOGI_OK, userarg);
        }
      });
    }
    else {
      brn->ForeachDiscoveredBranch([&](auto& tmp_uuid) {
        if (uuid) {
          memcpy(uuid, &tmp_uuid, tmp_uuid.size());
        }

        fn(YOGI_OK, userarg);
      });
    }

    if (buffer_too_small) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchAwaitDiscoveredBranchesChange(void* branch, void* uuid,
                                                      char* json, int jsonsize,
                                                      void (*fn)(int, void*),
                                                      void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchCancelAwaitDiscoveredBranchesChange(void* branch) {
  CHECK_PARAM(branch != nullptr);

  try {
  }
  CATCH_AND_RETURN;
}

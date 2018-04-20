#include "config.h"
#include "../include/yogi_core.h"
#include "api/constants.h"
#include "api/error.h"
#include "objects/context.h"
#include "objects/branch.h"
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

#define CATCH_AND_RETURN          \
  catch (const api::Error& err) { \
    return err.error_code();      \
  }                               \
  catch (const std::bad_alloc&) { \
    return YOGI_ERR_BAD_ALLOC;    \
  }                               \
  catch (...) {                   \
    return YOGI_ERR_UNKNOWN;      \
  }                               \
  return YOGI_OK;

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
    api::ObjectRegister::DestroyAll();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_CreateContext(void** context) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = objects::Context::Create();
    *context = api::ObjectRegister::Register(ctx);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_CreateBranch(void** branch, void* context,
                               const char* netname, const char* interface,
                               int advport, int advint) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(netname == nullptr || *netname != '\0');
  CHECK_PARAM(advport >= 0);
  CHECK_PARAM(advint >= 0);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);

    auto final_netname = netname ? std::string(netname) : utils::GetHostname();
    auto final_interface = interface ? interface : api::kDefaultInterface;
    auto final_advport = advport ? advport : api::kDefaultAdvPort;
    auto final_advint = advint ? advint : api::kDefaultAdvInterval;

    auto brn = objects::Branch::Create(ctx, final_netname, final_interface,
                                       final_advport,
                                       std::chrono::milliseconds(final_advint));
    *branch = api::ObjectRegister::Register(brn);
  }
  CATCH_AND_RETURN;
}

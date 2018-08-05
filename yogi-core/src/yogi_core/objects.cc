#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../api/object.h"
#include "../objects/context.h"

#include <boost/algorithm/string.hpp>

YOGI_API int YOGI_FormatObject(void* obj, char* str, int strsize,
                               const char* objfmt, const char* nullstr) {
  CHECK_PARAM(str != nullptr);
  CHECK_PARAM(strsize > 0);

  try {
    std::string s;
    if (obj == nullptr) {
      s = nullstr ? nullstr : api::kDefaultInvalidHandleString;
    } else {
      s = objfmt ? objfmt : api::kDefaultObjectFormat;

      auto object = api::ObjectRegister::Get(obj);
      boost::replace_all(s, "$T", object->TypeName());

      char buf[24];
      sprintf(buf, "%llx", reinterpret_cast<unsigned long long>(obj));
      boost::replace_all(s, "$x", buf);
      sprintf(buf, "%llX", reinterpret_cast<unsigned long long>(obj));
      boost::replace_all(s, "$X", buf);
    }

    if (!CopyStringToUserBuffer(s, str, strsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
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

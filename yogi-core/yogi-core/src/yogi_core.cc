#include "stdafx.h"
#include "../../include/yogi_core.h"
#include "../../yogi-core-lib/src/errors.h"


YOGI_API const char* YOGI_GetVersion()
{
  return YOGI_VERSION;
}

YOGI_API const char* YOGI_GetErrorString(int err)
{
  return getErrorString(err);
}

#include "stdafx.h"
#include <yogi_core.h>
#include "../src/errors.h"


int main(int argc, const char* argv[])
{
    getErrorString(34);
    YOGI_GetErrorString(344);
    YOGI_GetVersion();
    return 0;
}

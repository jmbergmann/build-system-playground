#pragma once

// MSVC-specific
#ifdef _MSC_VER
# define _SCL_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
# define _WINSOCK_DEPRECATED_NO_WARNINGS
# pragma warning (disable: 4250 4503)
#endif

// Include the main DLL header file for the error codes
#ifndef YOGI_API
# ifdef _MSC_VER
#   define YOGI_API __declspec(dllexport)
# else
#   define YOGI_API __attribute__((visibility("default")))
# endif
#endif

#include "../include/yogi_core.h"

// Debug & development
#ifndef NDEBUG
# include <stdio.h>
# define YOGI_ASSERT(x) {                                      \
    if (!(x)) {                                                \
      fprintf(stderr, "ASSERTION \"" #x "\" in %s:%d FAILED.", \
        __FILE__, __LINE__);                                   \
        *(char*)1 = 0;                                         \
    }                                                          \
  }
#else
# define YOGI_ASSERT(x)
#endif

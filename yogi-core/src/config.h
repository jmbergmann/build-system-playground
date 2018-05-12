#pragma once

// MSVC-specific
#ifdef _MSC_VER
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
#ifdef _MSC_VER
# define YOGI_DEBUG_BRAKE __debugbreak();
#else
# include <assert.h>
# define YOGI_DEBUG_BRAKE assert(false);
#endif

#ifndef NDEBUG
# include <iostream>
# define YOGI_TRACE std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
# define YOGI_ASSERT(x)                                                      \
  {                                                                          \
    if (!(x)) {                                                              \
      std::cerr << "ASSERTION \"" #x "\" in " << __FILE__ << ":" << __LINE__ \
                << " FAILED." << std::endl;                                  \
      YOGI_DEBUG_BRAKE;                                                      \
    }                                                                        \
  }
#else
# define YOGI_ASSERT(x)
#endif

#pragma once

// Include the main DLL header file for the error codes
#ifndef YOGI_API
# ifdef _MSC_VER
#   define YOGI_API __declspec(dllexport)
# else
#   define YOGI_API __attribute__((visibility("default")))
# endif
#endif

#include "../include/yogi_core.h"

// MSVC-specific
#if defined(_MSC_VER)
# define _WIN32_WINNT _WIN32_WINNT_VISTA
# define _SCL_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
# define _WINSOCK_DEPRECATED_NO_WARNINGS
# pragma warning (disable: 4250 4503)
#endif

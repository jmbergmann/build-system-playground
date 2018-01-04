#ifndef YOGI_CORE_H
#define YOGI_CORE_H

#define YOGI_VERSION "0.0.3"
#define YOGI_VERSION_MAJOR 0
#define YOGI_VERSION_MINOR 0
#define YOGI_VERSION_PATCH 3

#ifndef YOGI_API
# ifdef _MSC_VER
#   define YOGI_API __declspec(dllimport)
# else
#   define YOGI_API
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

YOGI_API const char* YOGI_GetVersion();
YOGI_API const char* YOGI_GetErrorString(int err);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // YOGI_CORE_H

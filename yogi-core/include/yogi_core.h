#ifndef YOGI_CORE_H
#define YOGI_CORE_H

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif // YOGI_CORE_H

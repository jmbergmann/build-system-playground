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

YOGI_API int YOGI_CreateContext(void** context, int flags);

YOGI_API int YOGI_Destroy(void* object);

// advertise locally and via UDP IPv6 multicast
// advport parameter is the UDP port that is used for advertising
// advint parameter is the interval between advertising beacons in milliseconds
// netname parameter is used to differentiate networks (e.g. Johannes' network vs Alex's network)
YOGI_API int YOGI_CreateNode(void** node, void* context, const char* netname, int advport, int advint, int flags);

// advertise via UDP
// try to connect locally first
YOGI_API int YOGI_CreateLeaf(void** leaf, void* context, const char* netname, const char* path, int advport, int advint, int flags);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // YOGI_CORE_H

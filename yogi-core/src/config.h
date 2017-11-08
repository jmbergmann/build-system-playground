#pragma once

#ifndef YOGI_API
# ifdef _MSC_VER
#   define YOGI_API __declspec(dllexport)
# else
#   define YOGI_API __attribute__((visibility("default")))
# endif
#endif

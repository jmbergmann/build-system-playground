#pragma once

#define YOGI_VERSION "0.0.3-alpha"

#ifndef YOGI_API
#	ifdef _MSC_VER
#	  define YOGI_API __declspec(dllexport)
#	else
#	  define YOGI_API __attribute__((visibility("default")))
#	endif
#endif

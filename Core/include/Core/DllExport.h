#pragma once

// Define EXPORTED for any platform
#if defined _WIN32 || defined __CYGWIN__
#ifdef WIN_EXPORT
  // Exporting...
#ifdef __GNUC__
#define CORE_API  __attribute__ ((dllexport))
#else
#define CORE_API __declspec(dllexport)    // Note: actually gcc seems to also supports this syntax.
#endif
#else
#ifdef __GNUC__
#define CORE_API __attribute__((dllimport))
#else
#define CORE_API __declspec(dllimport)    // Note: actually gcc seems to also supports this syntax.
#endif
#endif
#define NOT_EXPORTED
#else
#if __GNUC__ >= 4
#define CORE_API __attribute__((visibility("default")))
#define NOT_EXPORTED  __attribute__ ((visibility ("hidden")))
#else
#define CORE_API
#define NOT_EXPORTED
#endif
#endif
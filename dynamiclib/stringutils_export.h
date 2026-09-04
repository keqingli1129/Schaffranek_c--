#pragma once

// Symbol visibility for the shared library. STRINGUTILS_BUILDING is defined
// by CMake only while compiling the library itself.
#if defined(_WIN32)
#  if defined(STRINGUTILS_BUILDING)
#    define STRINGUTILS_API __declspec(dllexport)
#  else
#    define STRINGUTILS_API __declspec(dllimport)
#  endif
#else
#  define STRINGUTILS_API __attribute__((visibility("default")))
#endif

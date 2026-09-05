#pragma once

// Symbol visibility for the shared library. IMAGEUTILS_BUILDING is defined
// by CMake only while compiling the library itself.
#if defined(_WIN32)
#  if defined(IMAGEUTILS_BUILDING)
#    define IMAGEUTILS_API __declspec(dllexport)
#  else
#    define IMAGEUTILS_API __declspec(dllimport)
#  endif
#else
#  define IMAGEUTILS_API __attribute__((visibility("default")))
#endif

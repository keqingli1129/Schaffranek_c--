# imageutils: an OpenCV-backed shared library in dynamiclib

Date: 2026-09-04
Status: approved, not yet implemented

## Purpose

Add OpenCV to this project as a second shared library, `imageutils`, living beside
`stringutils` in `dynamiclib/`. This is a learning project: the goal is to exercise
the shared-library pattern against a real third-party dependency, and to demonstrate
keeping that dependency private to the library. The image algorithms themselves are
deliberately simple.

`stringutils` is not modified. `mathutils` is not modified.

## Environment

OpenCV 4.6.0 is already installed system-wide (Ubuntu `libopencv-dev`), with its CMake
package config at `/usr/lib/x86_64-linux-gnu/cmake/opencv4`. No vendoring, no
FetchContent, no submodule. `find_package(OpenCV REQUIRED)` resolves it as-is.

## Architecture

One new SHARED target, `imageutils`, built from `dynamiclib/imageutils.cpp`.

The central decision: **OpenCV does not appear in the public header.** `imageutils.h`
includes only `<memory>` and `<string>`. `cv::` symbols appear in exactly one file,
`imageutils.cpp`. This is achieved with the pimpl idiom — the public `Image` class
holds a `std::unique_ptr<Impl>`, and `Impl` (defined in the `.cpp`) holds the `cv::Mat`.

Consequences, all intended:

- OpenCV links **PRIVATE** to `imageutils`. `main.cpp` and the test executable never
  see an OpenCV include directory and never link OpenCV directly.
- The library's ABI does not depend on OpenCV's `cv::Mat` layout, so an OpenCV point
  upgrade does not force consumers to recompile.
- This is what the existing `CXX_VISIBILITY_PRESET hidden` + export-macro setup in
  `dynamiclib/` is for; `imageutils` follows that same discipline rather than inventing
  a new one.

The cost is a small amount of pimpl machinery: an out-of-line destructor, explicit
move operations, and an internal accessor so the free functions can reach the `cv::Mat`
inside a `const Image&`.

## Files

New:

- `dynamiclib/imageutils_export.h` — `IMAGEUTILS_API` macro. A direct mirror of
  `stringutils_export.h`: `__declspec(dllexport/dllimport)` on `_WIN32`,
  `__attribute__((visibility("default")))` elsewhere, keyed off `IMAGEUTILS_BUILDING`.
- `dynamiclib/imageutils.h` — public API below.
- `dynamiclib/imageutils.cpp` — implementation; the only file including OpenCV.
- `test/CMakeLists.txt`
- `test/imageutils_test.cpp`

Modified:

- `dynamiclib/CMakeLists.txt` — `find_package` plus the second target.
- `CMakeLists.txt` — `add_subdirectory(test)`.
- `main.cpp` — a short demo section.

### Test location

The project has no existing test convention. A mirrored top-level `test/` tree is used
rather than co-locating tests inside `dynamiclib/`, so that test sources stay out of the
library directory that gets built and installed. `test/` is flat for now, matching the
flat source layout, and has room for `stringutils`/`mathutils` tests later.

## Public API

```cpp
// dynamiclib/imageutils.h
#pragma once

#include <memory>
#include <string>

#include "imageutils_export.h"

namespace imageutils {

namespace detail { struct Access; }

// Owns a decoded image. Move-only: copying is deliberately disabled so that an
// accidental pass-by-value never silently deep-copies pixel data.
class IMAGEUTILS_API Image {
public:
    Image();
    ~Image();                                   // out-of-line: Impl is incomplete here
    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Decodes an image file. Returns false and leaves the object empty on failure.
    bool load(const std::string& path);

    // Encodes to a file; format is chosen from the extension. False on failure,
    // including when the image is empty.
    bool save(const std::string& path) const;

    bool empty() const;
    int  width() const;      // 0 when empty
    int  height() const;     // 0 when empty
    int  channels() const;   // 0 when empty

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend struct detail::Access;
};

// Synthesized 3-channel BGR pattern: blue ramps with x, green ramps with y, red is a
// fixed-size checkerboard. Deterministic, so tests can assert on it. Exists so the demo
// and the tests need no image files on disk. Empty Image if width or height <= 0.
IMAGEUTILS_API Image makeTestPattern(int width, int height);

// Single-channel copy of src. Already-grayscale input is passed through.
IMAGEUTILS_API Image toGrayscale(const Image& src);

// Resized copy. Returns empty if width or height <= 0.
IMAGEUTILS_API Image resize(const Image& src, int width, int height);

// Gaussian blur. kernelSize is forced odd and >= 1 (even values are incremented).
IMAGEUTILS_API Image blur(const Image& src, int kernelSize);

// Runtime OpenCV version string, e.g. "4.6.0".
IMAGEUTILS_API std::string openCvVersion();

}  // namespace imageutils
```

## Error handling

No exception may cross the library boundary. The `.so` is compiled against OpenCV;
consumers are not, so an escaping `cv::Exception` would be undecodable at the call site.

- `load` and `save` return `bool`.
- Every transform returns an **empty `Image`** on any failure: empty source, invalid
  dimensions, or an OpenCV throw.
- `imageutils.cpp` wraps each OpenCV call site in `try` / `catch (const cv::Exception&)`
  and converts to the empty-`Image` / `false` result.
- Accessors on an empty image return 0 rather than trapping.

There is no error-message channel in this version. Callers distinguish success from
failure by `empty()` or the returned `bool`.

## Build configuration

`dynamiclib/CMakeLists.txt` gains:

```cmake
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_library(imageutils SHARED imageutils.cpp)

target_include_directories(imageutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(imageutils PUBLIC cxx_std_23)

target_link_libraries(imageutils PRIVATE
    opencv_core opencv_imgproc opencv_imgcodecs)

set_target_properties(imageutils PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

target_compile_definitions(imageutils PRIVATE IMAGEUTILS_BUILDING)
```

Named component targets are linked rather than the catch-all `${OpenCV_LIBS}`, so the
binary depends only on the three modules actually used. `cxx_std_23` matches the
existing `stringutils` and `mathutils` targets.

Root `CMakeLists.txt`: `add_subdirectory(test)` after the existing subdirectories, and
`imageutils` added to the executable's `target_link_libraries`.

## Testing

One CTest executable, `test/imageutils_test.cpp`, using plain `assert` — no new
dependency. It links `imageutils` only, which also serves as a standing check that the
PRIVATE OpenCV link really is private: if OpenCV ever leaks into the public header, this
target fails to compile.

Cases:

1. `makeTestPattern(64, 48)` — not empty, width 64, height 48, channels 3.
2. `makeTestPattern` with a non-positive dimension — empty.
3. `toGrayscale` — channels 1, dimensions unchanged.
4. `toGrayscale` applied twice — still 1 channel, no failure.
5. `resize(src, 32, 16)` — exactly 32x16.
6. `resize` to a non-positive dimension — empty.
7. `blur(src, 5)` — dimensions and channel count preserved.
8. `blur` with an even kernel (e.g. 4) — succeeds, not empty.
9. Round trip: `save` a pattern to a temp path under the build directory, `load` it into
   a fresh `Image`, compare dimensions. Temp file removed afterwards.
10. `load` of a path that does not exist — returns false, image empty.
11. Every transform applied to a default-constructed `Image` — returns empty, no crash.
12. `openCvVersion()` — non-empty string.

Registered as `add_test(NAME imageutils_test COMMAND imageutils_test)`, runnable via
`ctest` from the build directory. The test writes only into its own working directory.

## Demo in main.cpp

Appended after the existing `stringutils` section, in the same print style:

```
opencv     = 4.6.0
pattern    = 256x128, 3 channels
grayscale  = 256x128, 1 channels
resized    = 64x32, 1 channels
blurred    = 64x32, 1 channels
saved      = imageutils_demo.png (yes)
```

It builds the pattern in memory, chains grayscale → resize → blur without touching disk
(the point of the in-memory `Image` handle), then writes one PNG into the working
directory as proof that `imgcodecs` is wired up.

## Out of scope

- `highgui` / `cv::imshow`. It needs a display and would block `main` on a keypress,
  which breaks headless runs.
- Copyable `Image` / an explicit `clone()`. Nothing in this design needs one; move-only
  is sufficient and cheaper to reason about.
- Video, camera capture, DNN, any OpenCV module beyond core/imgproc/imgcodecs.
- Installing or packaging `imageutils` via CPack beyond what the existing top-level
  CPack block already does.
- Tests for `stringutils` or `mathutils`. The new `test/` directory makes them easy to
  add later, but adding them is not part of this work.

## Success criteria

- `cmake --build` succeeds from a clean build directory with no warnings introduced.
- `ctest` runs `imageutils_test` and it passes.
- `grep -r "cv::\|#include.*opencv" dynamiclib/imageutils.h test/ main.cpp` returns nothing.
  (Not a bare `opencv` search — `main.cpp` prints an output label spelled `"opencv     = "`.)
- The demo section of `main.cpp` prints the values above and produces a valid PNG.
- `stringutils.h`, `stringutils.cpp`, `mathutils.*` are byte-identical to their state
  before this work.

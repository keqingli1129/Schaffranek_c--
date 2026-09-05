# imageutils OpenCV Shared Library Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `imageutils`, a second SHARED library in `dynamiclib/`, that wraps OpenCV behind a pimpl handle so no OpenCV symbol or header reaches any consumer.

**Architecture:** `imageutils.h` declares a move-only `Image` class holding `std::unique_ptr<Impl>`; `Impl` (defined only in `imageutils.cpp`) holds the `cv::Mat`. Free functions take `const Image&` and return `Image`, reaching the matrix through an internal `detail::Access` friend struct. OpenCV links PRIVATE, so `main.cpp` and the test executable compile with no OpenCV include path. Errors never escape as exceptions: `load`/`save` return `bool`, transforms return an empty `Image`.

**Tech Stack:** C++23, CMake >= 3.22, system OpenCV 4.6.0 (`core`, `imgproc`, `imgcodecs`) found via `find_package(OpenCV)`. No new third-party dependencies. Tests are a plain-C++ CTest executable with no framework.

**Spec:** `docs/superpowers/specs/2026-09-04-imageutils-opencv-design.md`

## Global Constraints

- `stringutils.h`, `stringutils.cpp`, `stringutils_export.h`, `mathutils.h`, `mathutils.cpp` must end byte-identical to their state before this work. Do not touch them.
- OpenCV must not appear in `dynamiclib/imageutils.h`, `test/`, or `main.cpp`. Verification: `grep -rn "opencv\|cv::" dynamiclib/imageutils.h test/ main.cpp` returns nothing.
- OpenCV links `PRIVATE` to `imageutils`. Never `PUBLIC`, never `INTERFACE`.
- Link the named component targets `opencv_core opencv_imgproc opencv_imgcodecs`. Do not use `${OpenCV_LIBS}`.
- Only `core`, `imgproc`, `imgcodecs` may be used. No `highgui`, no `cv::imshow`, no video, no DNN.
- No exception may cross the library boundary. Every OpenCV call site in `imageutils.cpp` is wrapped in `try` / `catch (const cv::Exception&)`.
- New targets use `target_compile_features(... PUBLIC cxx_std_23)`, matching the existing `stringutils` and `mathutils` targets.
- `imageutils` uses the same hardening as `stringutils`: `CXX_VISIBILITY_PRESET hidden`, `VISIBILITY_INLINES_HIDDEN ON`, `VERSION ${PROJECT_VERSION}`, `SOVERSION 0`, and `IMAGEUTILS_BUILDING` defined PRIVATE.
- Tests use a `CHECK` macro, never `assert` — `assert` compiles out under `NDEBUG` in Release builds and would make the test silently pass.
- The test writes only into its current working directory and removes what it writes.

## File Structure

| File | Status | Responsibility |
|---|---|---|
| `dynamiclib/imageutils_export.h` | create | `IMAGEUTILS_API` visibility macro. Mirror of `stringutils_export.h`. |
| `dynamiclib/imageutils.h` | create | Public API. Includes only `<memory>` and `<string>`. |
| `dynamiclib/imageutils.cpp` | create | Implementation. The only file in the repo that includes OpenCV. |
| `dynamiclib/CMakeLists.txt` | modify | Add `find_package(OpenCV ...)` and the `imageutils` target. Leave the `stringutils` block untouched. |
| `test/CMakeLists.txt` | create | Registers `imageutils_test` with CTest. |
| `test/imageutils_test.cpp` | create | All 12 test cases from the spec. |
| `CMakeLists.txt` | modify | `add_subdirectory(test)`; add `imageutils` to the executable's link list. |
| `main.cpp` | modify | Demo section appended after the existing `stringutils` output. |

## Build & Test Commands

Configure and build (used by every task):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

Run the tests:

```bash
ctest --test-dir build --output-on-failure
```

---

### Task 1: Build wiring, `Image` handle, and `openCvVersion()`

Establishes the target, the pimpl skeleton, file I/O, and the accessors. No transforms yet.

**Files:**
- Create: `dynamiclib/imageutils_export.h`
- Create: `dynamiclib/imageutils.h`
- Create: `dynamiclib/imageutils.cpp`
- Create: `test/CMakeLists.txt`
- Create: `test/imageutils_test.cpp`
- Modify: `dynamiclib/CMakeLists.txt` (append; do not edit the existing `stringutils` lines)
- Modify: `CMakeLists.txt` (add one `add_subdirectory`)

**Interfaces:**
- Consumes: nothing.
- Produces: `imageutils::Image` (default ctor, `~Image`, move ctor, move assignment, `bool load(const std::string&)`, `bool save(const std::string&) const`, `bool empty() const`, `int width() const`, `int height() const`, `int channels() const`); `std::string imageutils::openCvVersion()`; `imageutils::detail::Access` with `static const cv::Mat& mat(const Image&)` and `static Image wrap(cv::Mat)`; CMake target `imageutils`; CTest target `imageutils_test`.

- [ ] **Step 1: Write the failing test**

Create `test/imageutils_test.cpp`:

```cpp
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

#include "imageutils.h"

namespace {

int g_failures = 0;

void check(bool condition, const char* expression, int line) {
    if (!condition) {
        std::cerr << "FAILED line " << line << ": " << expression << '\n';
        ++g_failures;
    }
}

}  // namespace

#define CHECK(expr) check((expr), #expr, __LINE__)

int main() {
    // Case 12: OpenCV is actually linked and reachable.
    CHECK(!imageutils::openCvVersion().empty());

    // Case 11 (partial): a default-constructed image is empty and reports zeros.
    const imageutils::Image blank;
    CHECK(blank.empty());
    CHECK(blank.width() == 0);
    CHECK(blank.height() == 0);
    CHECK(blank.channels() == 0);

    // Case 10: loading a path that does not exist fails and leaves the image empty.
    imageutils::Image missing;
    CHECK(!missing.load("this_file_does_not_exist_12345.png"));
    CHECK(missing.empty());

    // Saving an empty image fails rather than writing a file.
    CHECK(!blank.save("should_not_be_created.png"));

    // The handle is move-only and moves cleanly.
    imageutils::Image moved = std::move(missing);
    CHECK(moved.empty());

    if (g_failures == 0) {
        std::cout << "imageutils_test: all checks passed\n";
    }
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

Create `test/CMakeLists.txt`:

```cmake
add_executable(imageutils_test imageutils_test.cpp)

# Links imageutils ONLY. If OpenCV ever leaks into the public header, this
# target stops compiling -- that is the point.
target_link_libraries(imageutils_test PRIVATE imageutils)

add_test(NAME imageutils_test COMMAND imageutils_test)
```

Append to the root `CMakeLists.txt`, immediately after the existing `add_subdirectory(dynamiclib)` line:

```cmake
add_subdirectory(test)
```

- [ ] **Step 2: Run the build to verify it fails**

Run: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j`
Expected: FAIL. CMake errors with `Cannot find source file` / no target `imageutils`, or the compiler reports `imageutils.h: No such file or directory`.

- [ ] **Step 3: Write the export header**

Create `dynamiclib/imageutils_export.h`:

```cpp
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
```

- [ ] **Step 4: Write the public header**

Create `dynamiclib/imageutils.h`. Write the whole file now, including the declarations Tasks 2 and 3 implement — the header is the contract and splitting it across tasks would churn it:

```cpp
#pragma once

#include <memory>
#include <string>

#include "imageutils_export.h"

namespace imageutils {

namespace detail {
struct Access;
}  // namespace detail

// Owns a decoded image. Move-only: copying is deliberately disabled so that an
// accidental pass-by-value never silently deep-copies pixel data.
//
// OpenCV is an implementation detail. Nothing in this header names it, and
// consumers do not link it.
class IMAGEUTILS_API Image {
public:
    Image();
    ~Image();
    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Decodes an image file, always as 3-channel BGR. Returns false and leaves
    // the object empty on failure.
    bool load(const std::string& path);

    // Encodes to a file; the format is chosen from the extension. Returns false
    // on failure, including when the image is empty.
    bool save(const std::string& path) const;

    bool empty() const;
    int width() const;     // 0 when empty
    int height() const;    // 0 when empty
    int channels() const;  // 0 when empty

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend struct detail::Access;
};

// Synthesized 3-channel BGR pattern: blue ramps with x, green ramps with y, red
// is a 16-pixel checkerboard. Deterministic, so tests can assert on it. Exists so
// the demo and the tests need no image files on disk. Empty Image if width or
// height <= 0.
IMAGEUTILS_API Image makeTestPattern(int width, int height);

// Single-channel copy of src. Already-grayscale input is passed through.
IMAGEUTILS_API Image toGrayscale(const Image& src);

// Resized copy. Returns an empty Image if width or height <= 0.
IMAGEUTILS_API Image resize(const Image& src, int width, int height);

// Gaussian blur. kernelSize is forced odd and >= 1; even values are incremented.
IMAGEUTILS_API Image blur(const Image& src, int kernelSize);

// Runtime OpenCV version string, e.g. "4.6.0".
IMAGEUTILS_API std::string openCvVersion();

}  // namespace imageutils
```

- [ ] **Step 5: Write the implementation for this task**

Create `dynamiclib/imageutils.cpp` with the pimpl plumbing, the accessors, file I/O, and `openCvVersion`. Tasks 2 and 3 append the free functions to this same file:

```cpp
#include "imageutils.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace imageutils {

struct Image::Impl {
    cv::Mat mat;
};

namespace detail {

// The bridge between the OpenCV-free public API and the cv::Mat inside.
struct Access {
    static const cv::Mat& mat(const Image& image) {
        static const cv::Mat kEmpty;
        return image.impl_ ? image.impl_->mat : kEmpty;
    }

    static Image wrap(cv::Mat mat) {
        Image image;
        image.impl_->mat = std::move(mat);
        return image;
    }
};

}  // namespace detail

Image::Image() : impl_(std::make_unique<Impl>()) {}

// Out-of-line: Impl is incomplete at the point of declaration in the header.
Image::~Image() = default;

// A moved-from Image has a null impl_; every accessor below tolerates that.
Image::Image(Image&&) noexcept = default;
Image& Image::operator=(Image&&) noexcept = default;

bool Image::load(const std::string& path) {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
    try {
        cv::Mat loaded = cv::imread(path, cv::IMREAD_COLOR);
        if (loaded.empty()) {
            impl_->mat.release();
            return false;
        }
        impl_->mat = std::move(loaded);
        return true;
    } catch (const cv::Exception&) {
        impl_->mat.release();
        return false;
    }
}

bool Image::save(const std::string& path) const {
    if (empty()) {
        return false;
    }
    try {
        return cv::imwrite(path, impl_->mat);
    } catch (const cv::Exception&) {
        return false;
    }
}

bool Image::empty() const {
    return !impl_ || impl_->mat.empty();
}

int Image::width() const {
    return empty() ? 0 : impl_->mat.cols;
}

int Image::height() const {
    return empty() ? 0 : impl_->mat.rows;
}

int Image::channels() const {
    return empty() ? 0 : impl_->mat.channels();
}

std::string openCvVersion() {
    return cv::getVersionString();
}

}  // namespace imageutils
```

- [ ] **Step 6: Wire up the build**

Append to `dynamiclib/CMakeLists.txt`, below the existing `stringutils` block. Do not modify any existing line in that file:

```cmake
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_library(imageutils SHARED imageutils.cpp)

target_include_directories(imageutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(imageutils PUBLIC cxx_std_23)

# PRIVATE keeps OpenCV's headers and libraries off every consumer of imageutils.
target_link_libraries(imageutils PRIVATE
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
)

# Export only what is marked IMAGEUTILS_API.
set_target_properties(imageutils PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

# Selects dllexport over dllimport while building the library itself.
target_compile_definitions(imageutils PRIVATE IMAGEUTILS_BUILDING)
```

- [ ] **Step 7: Run the test to verify it passes**

Run: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j && ctest --test-dir build --output-on-failure`
Expected: PASS. `imageutils_test` prints `imageutils_test: all checks passed`; ctest reports `100% tests passed, 0 tests failed out of 1`.

- [ ] **Step 8: Verify OpenCV did not leak**

Run: `grep -rn "opencv\|cv::" dynamiclib/imageutils.h test/ main.cpp; echo "exit=$?"`
Expected: no matching lines, `exit=1`.

Run: `ldd build/test/imageutils_test | grep -c opencv`
Expected: `0` — the test binary has no direct OpenCV dependency of its own.

- [ ] **Step 9: Commit**

```bash
git add dynamiclib/imageutils_export.h dynamiclib/imageutils.h dynamiclib/imageutils.cpp \
        dynamiclib/CMakeLists.txt test/CMakeLists.txt test/imageutils_test.cpp CMakeLists.txt
git commit -m "feat: add imageutils shared library with OpenCV hidden behind pimpl"
```

---

### Task 2: `makeTestPattern` and the save/load round trip

**Files:**
- Modify: `dynamiclib/imageutils.cpp` (add `makeTestPattern` above `openCvVersion`)
- Modify: `test/imageutils_test.cpp` (add cases before the final `if (g_failures == 0)` block)

**Interfaces:**
- Consumes: `imageutils::Image`, `imageutils::detail::Access::wrap`, `Image::load`, `Image::save` from Task 1.
- Produces: `imageutils::Image makeTestPattern(int width, int height)` — a `width` x `height`, 3-channel BGR image; empty when either dimension is <= 0.

- [ ] **Step 1: Write the failing test**

Add to `test/imageutils_test.cpp`, inside `main`, after the Task 1 checks and before the `if (g_failures == 0)` block:

```cpp
    // Case 1: the pattern has the requested geometry.
    const imageutils::Image pattern = imageutils::makeTestPattern(64, 48);
    CHECK(!pattern.empty());
    CHECK(pattern.width() == 64);
    CHECK(pattern.height() == 48);
    CHECK(pattern.channels() == 3);

    // Case 2: non-positive dimensions yield an empty image, not a crash.
    CHECK(imageutils::makeTestPattern(0, 10).empty());
    CHECK(imageutils::makeTestPattern(10, -1).empty());

    // Case 9: save/load round trip preserves geometry.
    const std::string roundTripPath = "imageutils_test_roundtrip.png";
    CHECK(pattern.save(roundTripPath));
    imageutils::Image reloaded;
    CHECK(reloaded.load(roundTripPath));
    CHECK(reloaded.width() == pattern.width());
    CHECK(reloaded.height() == pattern.height());
    CHECK(reloaded.channels() == 3);
    std::remove(roundTripPath.c_str());
```

Add `#include <cstdio>` to the includes at the top of the file for `std::remove`.

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build -j`
Expected: FAIL at link. Task 1 already declared `makeTestPattern` in the header, so compilation succeeds and the linker reports `undefined reference to imageutils::makeTestPattern(int, int)`.

- [ ] **Step 3: Write the implementation**

Add to `dynamiclib/imageutils.cpp`. Add `#include <algorithm>` to the include block first, then insert this function after `Image::channels()` and before `openCvVersion()`:

```cpp
Image makeTestPattern(int width, int height) {
    if (width <= 0 || height <= 0) {
        return Image{};
    }
    try {
        cv::Mat mat(height, width, CV_8UC3);
        const int lastX = std::max(width - 1, 1);
        const int lastY = std::max(height - 1, 1);
        for (int y = 0; y < height; ++y) {
            auto* row = mat.ptr<cv::Vec3b>(y);
            for (int x = 0; x < width; ++x) {
                const auto blue = static_cast<unsigned char>((x * 255) / lastX);
                const auto green = static_cast<unsigned char>((y * 255) / lastY);
                const bool lightSquare = (((x / 16) + (y / 16)) % 2) == 0;
                // cv::Vec3b is ordered B, G, R.
                row[x] = cv::Vec3b(blue, green, lightSquare ? 255 : 0);
            }
        }
        return detail::Access::wrap(std::move(mat));
    } catch (const cv::Exception&) {
        return Image{};
    }
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cmake --build build -j && ctest --test-dir build --output-on-failure`
Expected: PASS, `100% tests passed`. No `imageutils_test_roundtrip.png` is left behind — confirm with `ls build/test/imageutils_test_roundtrip.png`, expected `No such file or directory`.

- [ ] **Step 5: Commit**

```bash
git add dynamiclib/imageutils.cpp test/imageutils_test.cpp
git commit -m "feat: add makeTestPattern and cover the save/load round trip"
```

---

### Task 3: The transforms — `toGrayscale`, `resize`, `blur`

**Files:**
- Modify: `dynamiclib/imageutils.cpp` (add three functions after `makeTestPattern`)
- Modify: `test/imageutils_test.cpp` (add cases before the final `if (g_failures == 0)` block)

**Interfaces:**
- Consumes: `imageutils::Image`, `detail::Access::mat`, `detail::Access::wrap`, `makeTestPattern` from Tasks 1-2.
- Produces: `Image toGrayscale(const Image&)`, `Image resize(const Image&, int, int)`, `Image blur(const Image&, int)`. Each returns an empty `Image` on empty input or invalid arguments.

- [ ] **Step 1: Write the failing test**

Add to `test/imageutils_test.cpp`, after the Task 2 checks and before the `if (g_failures == 0)` block. It reuses the `pattern` variable declared in Task 2:

```cpp
    // Case 3: grayscale collapses to one channel, geometry unchanged.
    const imageutils::Image gray = imageutils::toGrayscale(pattern);
    CHECK(!gray.empty());
    CHECK(gray.channels() == 1);
    CHECK(gray.width() == pattern.width());
    CHECK(gray.height() == pattern.height());

    // Case 4: grayscaling an already-grayscale image is a no-op, not an error.
    const imageutils::Image grayTwice = imageutils::toGrayscale(gray);
    CHECK(!grayTwice.empty());
    CHECK(grayTwice.channels() == 1);
    CHECK(grayTwice.width() == gray.width());

    // Case 5: resize hits the requested size exactly.
    const imageutils::Image small = imageutils::resize(gray, 32, 16);
    CHECK(small.width() == 32);
    CHECK(small.height() == 16);
    CHECK(small.channels() == 1);

    // Case 6: non-positive targets yield an empty image.
    CHECK(imageutils::resize(pattern, 0, 16).empty());
    CHECK(imageutils::resize(pattern, 32, -5).empty());

    // Case 7: blur preserves geometry and channel count.
    const imageutils::Image blurred = imageutils::blur(small, 5);
    CHECK(blurred.width() == small.width());
    CHECK(blurred.height() == small.height());
    CHECK(blurred.channels() == small.channels());

    // Case 8: an even kernel is accepted and rounded up to odd.
    CHECK(!imageutils::blur(small, 4).empty());
    CHECK(!imageutils::blur(small, 0).empty());

    // Case 11: every transform tolerates an empty input.
    const imageutils::Image none;
    CHECK(imageutils::toGrayscale(none).empty());
    CHECK(imageutils::resize(none, 8, 8).empty());
    CHECK(imageutils::blur(none, 3).empty());
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build -j`
Expected: FAIL at link with `undefined reference to imageutils::toGrayscale(imageutils::Image const&)` and the same for `resize` and `blur`.

- [ ] **Step 3: Write the implementation**

Add `#include <opencv2/imgproc.hpp>` to the include block in `dynamiclib/imageutils.cpp`, then add these three functions after `makeTestPattern` and before `openCvVersion`:

```cpp
Image toGrayscale(const Image& src) {
    if (src.empty()) {
        return Image{};
    }
    try {
        const cv::Mat& input = detail::Access::mat(src);
        if (input.channels() == 1) {
            return detail::Access::wrap(input.clone());
        }
        cv::Mat output;
        cv::cvtColor(input, output, cv::COLOR_BGR2GRAY);
        return detail::Access::wrap(std::move(output));
    } catch (const cv::Exception&) {
        return Image{};
    }
}

Image resize(const Image& src, int width, int height) {
    if (src.empty() || width <= 0 || height <= 0) {
        return Image{};
    }
    try {
        cv::Mat output;
        cv::resize(detail::Access::mat(src), output, cv::Size(width, height), 0, 0,
                   cv::INTER_AREA);
        return detail::Access::wrap(std::move(output));
    } catch (const cv::Exception&) {
        return Image{};
    }
}

Image blur(const Image& src, int kernelSize) {
    if (src.empty()) {
        return Image{};
    }
    // GaussianBlur requires a positive odd kernel.
    int kernel = kernelSize < 1 ? 1 : kernelSize;
    if (kernel % 2 == 0) {
        ++kernel;
    }
    try {
        cv::Mat output;
        cv::GaussianBlur(detail::Access::mat(src), output, cv::Size(kernel, kernel), 0);
        return detail::Access::wrap(std::move(output));
    } catch (const cv::Exception&) {
        return Image{};
    }
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cmake --build build -j && ctest --test-dir build --output-on-failure`
Expected: PASS, `100% tests passed, 0 tests failed out of 1`, with `imageutils_test: all checks passed` in the output.

- [ ] **Step 5: Commit**

```bash
git add dynamiclib/imageutils.cpp test/imageutils_test.cpp
git commit -m "feat: add toGrayscale, resize and blur transforms"
```

---

### Task 4: Demo in `main.cpp`

**Files:**
- Modify: `main.cpp` (add an include and a section before `return 0;`)
- Modify: `CMakeLists.txt` (add `imageutils` to `target_link_libraries`)

**Interfaces:**
- Consumes: the whole `imageutils` API from Tasks 1-3.
- Produces: nothing other tasks depend on.

- [ ] **Step 1: Link the library into the executable**

In the root `CMakeLists.txt`, change:

```cmake
target_link_libraries(Schaffranek_c++ PRIVATE mathutils stringutils)
```

to:

```cmake
target_link_libraries(Schaffranek_c++ PRIVATE mathutils stringutils imageutils)
```

- [ ] **Step 2: Write the demo**

Add `#include "imageutils.h"` to `main.cpp` alongside the existing `"mathutils.h"` and `"stringutils.h"` includes. Then insert this block after the `joined` line and before `return 0;`:

```cpp
    std::cout << "opencv     = " << imageutils::openCvVersion() << '\n';

    const imageutils::Image pattern = imageutils::makeTestPattern(256, 128);
    std::cout << "pattern    = " << pattern.width() << 'x' << pattern.height()
              << ", " << pattern.channels() << " channels\n";

    // Chained in memory -- no disk round trip between steps.
    const imageutils::Image gray = imageutils::toGrayscale(pattern);
    std::cout << "grayscale  = " << gray.width() << 'x' << gray.height()
              << ", " << gray.channels() << " channels\n";

    const imageutils::Image resized = imageutils::resize(gray, 64, 32);
    std::cout << "resized    = " << resized.width() << 'x' << resized.height()
              << ", " << resized.channels() << " channels\n";

    const imageutils::Image blurred = imageutils::blur(resized, 5);
    std::cout << "blurred    = " << blurred.width() << 'x' << blurred.height()
              << ", " << blurred.channels() << " channels\n";

    const std::string outputPath = "imageutils_demo.png";
    const bool saved = blurred.save(outputPath);
    std::cout << "saved      = " << outputPath << " (" << (saved ? "yes" : "no") << ")"
              << std::endl;
```

- [ ] **Step 3: Build and run it**

Run: `cmake --build build -j && (cd build && echo "demo" | ./Schaffranek_c++)`
Expected output, after the existing mathutils/stringutils lines:

```
opencv     = 4.6.0
pattern    = 256x128, 3 channels
grayscale  = 256x128, 1 channels
resized    = 64x32, 1 channels
blurred    = 64x32, 1 channels
saved      = imageutils_demo.png (yes)
```

- [ ] **Step 4: Verify the PNG is real**

Run: `file build/imageutils_demo.png`
Expected: `PNG image data, 64 x 32, 8-bit grayscale, non-interlaced`.

- [ ] **Step 5: Final whole-branch verification**

```bash
ctest --test-dir build --output-on-failure
grep -rn "opencv\|cv::" dynamiclib/imageutils.h test/ main.cpp
git diff --stat main -- dynamiclib/stringutils.h dynamiclib/stringutils.cpp \
    dynamiclib/stringutils_export.h staticlab/mathutils.h staticlab/mathutils.cpp
```

Expected: tests pass; the grep prints nothing; the `git diff --stat` prints nothing, proving `stringutils` and `mathutils` are untouched.

- [ ] **Step 6: Commit**

```bash
rm -f build/imageutils_demo.png
git add main.cpp CMakeLists.txt
git commit -m "feat: demo imageutils from main"
```

---

## Coverage Check

| Spec requirement | Task |
|---|---|
| `imageutils_export.h` mirroring `stringutils_export.h` | 1 |
| Pimpl `Image`, move-only, out-of-line dtor | 1 |
| `load` / `save` / `empty` / `width` / `height` / `channels` | 1 |
| `openCvVersion` | 1 |
| CMake target, PRIVATE OpenCV, hidden visibility, SOVERSION | 1 |
| `test/` tree, CTest registration | 1 |
| `makeTestPattern` | 2 |
| Save/load round trip through a temp file | 2 |
| `toGrayscale`, `resize`, `blur` | 3 |
| No exception crosses the boundary | 1, 2, 3 (every OpenCV call site wrapped) |
| Empty-`Image` failure results | 1, 2, 3 |
| All 12 spec test cases | 1 (10, 11-partial, 12), 2 (1, 2, 9), 3 (3-8, 11) |
| `main.cpp` demo | 4 |
| `stringutils` / `mathutils` untouched | 4, Step 5 |

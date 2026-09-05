#include <cstdio>
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

    // Case 10b: loading a path that exists but is a directory fails and leaves
    // the image empty. "." always exists and is never a regular file.
    imageutils::Image directory;
    CHECK(!directory.load("."));
    CHECK(directory.empty());

    // Saving an empty image fails rather than writing a file.
    CHECK(!blank.save("should_not_be_created.png"));

    // The handle is move-only and moves cleanly.
    imageutils::Image moved = std::move(missing);
    CHECK(moved.empty());

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

    if (g_failures == 0) {
        std::cout << "imageutils_test: all checks passed\n";
    }
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

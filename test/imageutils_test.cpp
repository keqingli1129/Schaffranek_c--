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

    // Showing an empty image fails without ever touching the GUI, so this is
    // safe under a headless test runner. A non-empty image is not shown here:
    // show() blocks until a viewer dismisses the window.
    CHECK(!blank.show("should_not_be_shown"));

    // The handle is move-only and moves cleanly.
    imageutils::Image moved = std::move(missing);
    CHECK(moved.empty());
    // The moved-from source has a null impl_; every accessor must tolerate that.
    CHECK(missing.empty());
    CHECK(missing.width() == 0);
    CHECK(missing.height() == 0);
    CHECK(missing.channels() == 0);
    CHECK(!missing.save("should_not_be_created.png"));

    // Case 1: the pattern has the requested geometry.
    const imageutils::Image pattern = imageutils::makeTestPattern(64, 48);
    CHECK(!pattern.empty());
    CHECK(pattern.width() == 64);
    CHECK(pattern.height() == 48);
    CHECK(pattern.channels() == 3);

    // An unknown extension makes OpenCV's writer throw; the library must
    // convert that into an in-band false, not let it escape.
    CHECK(!pattern.save("imageutils_test_unwritable.zzz"));

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

    // HSV keeps the geometry and stays 3-channel.
    const imageutils::Image hsv = imageutils::toHsv(pattern);
    CHECK(!hsv.empty());
    CHECK(hsv.channels() == 3);
    CHECK(hsv.width() == pattern.width());
    CHECK(hsv.height() == pattern.height());

    // Single-channel input goes through BGR instead of failing.
    const imageutils::Image hsvFromGray = imageutils::toHsv(gray);
    CHECK(!hsvFromGray.empty());
    CHECK(hsvFromGray.channels() == 3);
    CHECK(hsvFromGray.width() == gray.width());

    // Showing an empty image as HSV fails before touching the GUI, so this is
    // safe under a headless runner, exactly like the show() check above.
    CHECK(!blank.showHsv("should_not_be_shown"));

    // adjustHsv returns a displayable BGR image, not HSV data, and keeps the
    // geometry. A grayscale source is widened to 3 channels on the way.
    const imageutils::Image shifted = imageutils::adjustHsv(pattern, 120, 1.5, 0.8);
    CHECK(!shifted.empty());
    CHECK(shifted.channels() == 3);
    CHECK(shifted.width() == pattern.width());
    CHECK(shifted.height() == pattern.height());
    CHECK(imageutils::adjustHsv(gray, 45).channels() == 3);

    // Out-of-range arguments are absorbed rather than rejected: the hue wraps
    // both ways, and a negative scale clamps to 0 (a full desaturate).
    CHECK(!imageutils::adjustHsv(pattern, 720).empty());
    CHECK(!imageutils::adjustHsv(pattern, -30).empty());
    CHECK(!imageutils::adjustHsv(pattern, 0, -1.0, -1.0).empty());

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

    // Playing a path that is not an existing file fails before any window is
    // created, so this is safe under a headless runner just like show() above.
    CHECK(!imageutils::playVideo("this_video_does_not_exist_12345.mp4"));
    CHECK(!imageutils::playVideo("."));

    // Case 9: drawCircle mutates in place and leaves the geometry alone. A
    // centre outside the image is clipped, not rejected -- the call still
    // succeeds because nothing about the request was wrong.
    imageutils::Image canvas = imageutils::makeTestPattern(64, 64);
    CHECK(canvas.drawCircle(32, 32, 10, imageutils::Color{0, 0, 255}));
    CHECK(canvas.width() == 64);
    CHECK(canvas.height() == 64);
    CHECK(canvas.channels() == 3);
    CHECK(canvas.drawCircle(-100, -100, 5));              // fully off-image
    CHECK(canvas.drawCircle(1000, 32, 4));                // off the right edge
    CHECK(canvas.drawCircle(32, 32, 0));                  // degenerate, allowed
    CHECK(canvas.drawCircle(32, 32, 12, imageutils::Color{255, 0, 0}, 2));
    CHECK(canvas.drawCircle(32, 32, 12, imageutils::Color{255, 0, 0}, -7));
    // Out-of-range components clamp rather than throwing or wrapping.
    CHECK(canvas.drawCircle(10, 10, 3, imageutils::Color{-40, 900, 12}));
    // Single-channel targets are fine; only the first component is used.
    imageutils::Image grayCanvas = imageutils::toGrayscale(canvas);
    CHECK(grayCanvas.drawCircle(16, 16, 6, imageutils::Color{255, 0, 0}));
    CHECK(grayCanvas.channels() == 1);

    // Case 10: a negative radius is the one argument error, and an empty image
    // has nothing to draw into.
    CHECK(!canvas.drawCircle(32, 32, -1));
    imageutils::Image blankCanvas;
    CHECK(!blankCanvas.drawCircle(0, 0, 5));

    // Case 11: every transform tolerates an empty input.
    const imageutils::Image none;
    CHECK(imageutils::toGrayscale(none).empty());
    CHECK(imageutils::toHsv(none).empty());
    CHECK(imageutils::adjustHsv(none, 90).empty());
    CHECK(imageutils::resize(none, 8, 8).empty());
    CHECK(imageutils::blur(none, 3).empty());

    if (g_failures == 0) {
        std::cout << "imageutils_test: all checks passed\n";
    }
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

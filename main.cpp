#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <string_view>
#include <vector>

#include "mathutils.h"
#include "stringutils.h"
#include "imageutils.h"

namespace {

// The demo's files live in the project folder, but the binary sits in build/
// and an IDE run button starts it in a directory of its own choosing. So look
// at the working directory and walk up to the folder holding CMakeLists.txt;
// that covers being launched from the project root or from build/ alike.
// Falls back to the working directory when there is nothing to find.
std::filesystem::path projectDir() {
    const std::filesystem::path cwd = std::filesystem::current_path();
    for (std::filesystem::path dir = cwd; dir != dir.parent_path(); dir = dir.parent_path()) {
        if (std::filesystem::exists(dir / "CMakeLists.txt")) {
            return dir;
        }
    }
    return cwd;
}

}  // namespace

int main(int argc, char** argv) {
    // std::string name;
    // std::cout << "Enter your name: ";
    // std::getline(std::cin, name);
    // std::cout << mathutils::greet(name) << std::endl;

    // const std::vector<int> values{3, 7, 11, 12, 42};

    // std::cout << "sum     = " << mathutils::sum(values) << '\n';
    // std::cout << "average = " << mathutils::average(values) << '\n';
    // std::cout << "5!      = " << mathutils::factorial(5) << '\n';

    // std::cout << "primes  =";
    // for (int value : values) {
    //     if (mathutils::isPrime(value)) {
    //         std::cout << ' ' << value;
    //     }
    // }
    // std::cout << std::endl;

    // // 2x + y = 5, x - 3y = -8  ->  x = 1, y = 3. Eigen does the work inside
    // // mathutils; nothing about it reaches this translation unit.
    // const std::vector<double> solution =
    //     mathutils::solveLinearSystem({{2.0, 1.0}, {1.0, -3.0}}, {5.0, -8.0});
    // std::cout << "solved  =";
    // for (double value : solution) {
    //     std::cout << ' ' << value;
    // }
    // std::cout << (solution.empty() ? " <no solution>" : "") << std::endl;

    // const std::string phrase = "  A man, a plan, a canal: Panama  ";
    // const std::string trimmed = stringutils::trim(phrase);

    // std::cout << "trimmed    = [" << trimmed << "]\n";
    // std::cout << "upper      = " << stringutils::toUpper(trimmed) << '\n';
    // std::cout << "lower      = " << stringutils::toLower(trimmed) << '\n';
    // std::cout << "reversed   = " << stringutils::reverse(trimmed) << '\n';
    // std::cout << "palindrome = " << (stringutils::isPalindrome(trimmed) ? "yes" : "no") << '\n';

    // // const std::vector<std::string> words = stringutils::split("alpha,beta,gamma", ',');
    // // std::cout << "joined     = " << stringutils::join(words, " | ") << std::endl;

    // std::cout << "opencv     = " << imageutils::openCvVersion() << '\n';

    // // Failures in imageutils are in-band: a transform returns an empty Image
    // // rather than throwing. Consumers check, they don't catch.
    // const auto report = [](const char* label, const imageutils::Image& image) {
    //     std::cout << label << " = " << image.width() << 'x' << image.height()
    //               << ", " << image.channels() << " channels\n";
    //     return !image.empty();
    // };

    // // Needed before the first load, and again later for the demo's outputs.
    // const std::filesystem::path project = projectDir();

    // const imageutils::Image pattern = imageutils::makeTestPattern(256, 128);
    // if (!report("pattern   ", pattern)) { return 1; }

    // // The HSV samples below run on the real screenshot from the source tree --
    // // a photograph responds to a hue rotation in a way a synthetic pattern
    // // cannot. It is tracked in git, but a checkout could still be missing it,
    // // so fall back to a pattern rather than failing the whole demo. Image is
    // // move-only, so the fallback moves in a fresh one; `pattern` cannot be
    // // copied here, which is the type doing its job.
    // const std::string sourcePath = (project / "Screenshot.png").string();
    // imageutils::Image source;
    // if (!source.load(sourcePath)) {
    //     std::cout << "src-path   = " << sourcePath << " not found, using pattern\n";
    //     source = imageutils::makeTestPattern(pattern.width(), pattern.height());
    // } else {
    //     std::cout << "src-path   = " << sourcePath << '\n';
    // }
    // if (!report("source    ", source)) { return 1; }

    // // Chained in memory -- no disk round trip between steps.
    // const imageutils::Image gray = imageutils::toGrayscale(pattern);
    // if (!report("grayscale ", gray)) { return 1; }

    // const imageutils::Image resized = imageutils::resize(gray, 64, 32);
    // if (!report("resized   ", resized)) { return 1; }

    // const imageutils::Image blurred = imageutils::blur(resized, 5);
    // if (!report("blurred   ", blurred)) { return 1; }

    // // adjustHsv() works in HSV but hands back a normal BGR image, so its result
    // // can be saved and shown like any other: +120 degrees of hue, saturation
    // // pushed up by half, brightness left alone.
    // const imageutils::Image shifted = imageutils::adjustHsv(source, 120, 1.5, 1.0);
    // if (!report("hue-shift ", shifted)) { return 1; }

    // // The same call with a zero saturation scale is a desaturate -- the colours
    // // collapse to grey while the image stays 3-channel BGR.
    // const imageutils::Image desaturated = imageutils::adjustHsv(source, 0, 0.0, 1.0);
    // if (!report("desaturate", desaturated)) { return 1; }

    // // Hue is given in real degrees and wraps both ways, so -240 names the same
    // // rotation as the +120 above and produces a byte-identical image. Anything
    // // outside 0-360 is absorbed the same way; 720 lands back on 0.
    // const imageutils::Image wrapped = imageutils::adjustHsv(source, -240, 1.5, 1.0);
    // if (!report("hue-wrap  ", wrapped)) { return 1; }

    // // valueScale is the brightness dial: below 1 darkens, above 1 brightens and
    // // clips at white instead of wrapping back around to black.
    // const imageutils::Image dimmed = imageutils::adjustHsv(source, 0, 1.0, 0.5);
    // if (!report("dimmed    ", dimmed)) { return 1; }

    // const imageutils::Image brightened = imageutils::adjustHsv(source, 0, 1.0, 1.8);
    // if (!report("brightened", brightened)) { return 1; }

    // // A grayscale source carries no hue to rotate, but the call still works:
    // // it is widened to 3-channel BGR first, so only valueScale shows up.
    // const imageutils::Image grayLifted = imageutils::adjustHsv(imageutils::toGrayscale(source), 200, 1.0, 1.3);
    // if (!report("gray-hsv  ", grayLifted)) { return 1; }

    // // 0 degrees with both scales at 1 changes nothing in HSV, but the result is
    // // still a BGR->HSV->BGR round trip, and 8-bit HSV cannot represent every
    // // BGR colour exactly (hue is halved into 0-179). So this is the visual
    // // baseline for the numbers above, not a bit-exact copy: on the screenshot
    // // about a third of the channel values move, by up to 3 levels out of 255.
    // const imageutils::Image unchanged = imageutils::adjustHsv(source, 0, 1.0, 1.0);
    // if (!report("unchanged ", unchanged)) { return 1; }

    // // Failure stays in-band here as everywhere else in imageutils: an empty
    // // source yields an empty result, so callers test instead of catching.
    // const imageutils::Image fromEmpty = imageutils::adjustHsv(imageutils::Image{}, 90);
    // std::cout << "empty-src  = " << (fromEmpty.empty() ? "empty, as expected" : "unexpected")
    //           << '\n';

    // // Worth looking at now that the source is a photograph. *.png is gitignored,
    // // so these land beside the screenshot without becoming repository noise.
    // const auto saveSample = [&project](const char* name, const imageutils::Image& image) {
    //     const std::string path =
    //         (project / ("imageutils_hsv_" + std::string(name) + ".png")).string();
    //     std::cout << "saved      = " << path << " (" << (image.save(path) ? "yes" : "no") << ")\n";
    // };
    // saveSample("shift", shifted);
    // saveSample("desaturated", desaturated);
    // saveSample("dimmed", dimmed);
    // saveSample("brightened", brightened);

    // const std::string outputPath = (project / "imageutils_demo.png").string();
    // const bool saved = blurred.save(outputPath);
    // std::cout << "saved      = " << outputPath << " (" << (saved ? "yes" : "no") << ")"
    //           << std::endl;

    // // Opt-in: both viewers block until dismissed and need a display, so they
    // // must never run in a headless build or a test harness. "--show" takes an
    // // optional path; without one it falls back to the image in the source tree.
    // bool wantCamera = false;
    // bool wantShow = false;
    // bool wantPlay = false;
    // std::string showPath = sourcePath;
    // // No sample video ships with the project, so "--play" has no fallback: the
    // // path is the argument's whole point.
    // std::string playPath;
    // for (int i = 1; i < argc; ++i) {
    //     const std::string_view arg(argv[i]);
    //     if (arg == "--camera") {
    //         wantCamera = true;
    //     } else if (arg == "--show") {
    //         wantShow = true;
    //         // A following argument that is not itself a flag is the path.
    //         if (i + 1 < argc && argv[i + 1][0] != '-') {
    //             showPath = argv[++i];
    //         }
    //     } else if (arg == "--play") {
    //         wantPlay = true;
    //         if (i + 1 < argc && argv[i + 1][0] != '-') {
    //             playPath = argv[++i];
    //         }
    //     }
    // }

    // if (wantShow) {
    //     imageutils::Image picture;
    //     if (!picture.load(showPath)) {
    //         std::cout << "show       = cannot load " << showPath << '\n';
    //         return 1;
    //     }
    //     if (!report("show      ", picture)) { return 1; }
    //     std::cout << "show       = " << showPath << " (any key to close)" << std::endl;
    //     if (!picture.show(showPath)) {
    //         std::cout << "show       = no display\n";
    //         return 1;
    //     }
    //     std::cout << "show       = closed\n";
    //     const imageutils::Image gray = imageutils::toGrayscale(picture);
    //     gray.show("Grayscale");
    //     std::cout << "hsv        = (any key to close)" << std::endl;
    //     picture.showHsv("HSV");
    //     // Same picture, hue rotated a quarter turn and a touch darker. Unlike
    //     // showHsv() above, this one is a real picture, not a channel dump.
    //     std::cout << "hue-shift  = +90 deg (any key to close)" << std::endl;
    //     imageutils::adjustHsv(picture, 90, 1.2, 0.9).show("Hue +90");
    // }

    // if (wantPlay && !playPath.empty()) {
    //     // The binary usually runs from build/, so a relative path that isn't
    //     // there is worth one retry against the project folder -- the same
    //     // reasoning that projectDir() exists for. An absolute path, or a
    //     // relative one that already resolves, is left exactly as given.
    //     std::error_code ec;
    //     const std::filesystem::path given(playPath);
    //     if (given.is_relative() && !std::filesystem::exists(given, ec)) {
    //         const std::filesystem::path inProject = (project / given).lexically_normal();
    //         if (std::filesystem::exists(inProject, ec)) {
    //             playPath = inProject.string();
    //         }
    //     }
    // }

    // if (wantPlay) {
    //     if (playPath.empty()) {
    //         std::cout << "play       = --play needs a video file path\n";
    //         return 1;
    //     }
    //     std::cout << "play       = " << playPath
    //               << " (Space pauses, Esc quits)" << std::endl;
    //     if (!imageutils::playVideo(playPath, playPath)) {
    //         std::cout << "play       = cannot play " << playPath << '\n';
    //         return 1;
    //     }
    //     std::cout << "play       = finished\n";
    // }

    // if (wantCamera) {
    //     std::cout << "camera     = opening (Esc to close)" << std::endl;
    //     // The preview displays only, so any Image will do as the receiver.
    //     const imageutils::Image live;
    //     if (!live.showCameraPreview(0, "Original")) {
    //         std::cout << "camera     = unavailable\n";
    //         return 1;
    //     }
    //     std::cout << "camera     = closed\n";
    // }

    // return saved ? 0 : 1;

    // Every adjustHsv knob, one window each. show() blocks until a key is
    // pressed, so the windows arrive one at a time in the order listed below.
    const std::filesystem::path project = projectDir();
    const std::string sourcePath = (project / "Screenshot.png").string();

    imageutils::Image source;
    if (!source.load(sourcePath)) {
        std::cout << "source     = cannot load " << sourcePath << '\n';
        return 1;
    }
    std::cout << "source     = " << sourcePath << " (" << source.width() << 'x'
              << source.height() << ")\n";
    // source.show("Source");
    source.showHsv("Source HSV");
    // imageutils::Image hsv = imageutils::toHsv(source);
    // hsv.show("HSV");
    imageutils::Image hsv = imageutils::adjustHsv(source, 0, 1.0, 1.0);
    hsv.show("Adjusted HSV");

    // bilateralFilter, against blur() for contrast. Both windows below smooth
    // by about the same amount; the difference to look for is what happens to
    // the edges -- blur() takes them with it, bilateralFilter() leaves them
    // standing while the flat areas go smooth. That is the whole reason to pay
    // for it, and it is only visible on a photograph, not a synthetic pattern.
    //
    // Each one is timed because the cost is the other half of the trade: the
    // neighbourhood is scanned per pixel with no separable shortcut, so this
    // runs orders of magnitude slower than the Gaussian and grows with the
    // square of the diameter.
    {
        // The arguments, plus the window title that names them. A diameter of
        // -1 is the "derive it from sigmaSpace" spelling, not a mistake.
        struct Smooth {
            const char* title;
            int diameter;
            double sigmaColor;
            double sigmaSpace;
        };
        const std::vector<Smooth> smooths{
            {"B2 - Bilateral d=5, sigma 50/50 (video-rate)",    5,  50.0,  50.0},
            {"B3 - Bilateral d=9, sigma 75/75 (the default)",   9,  75.0,  75.0},
            {"B4 - Bilateral d=9, sigmaColor 200 (too high)",   9, 200.0,  75.0},
            {"B5 - Bilateral d=-1, sigmaSpace 15 sets the size", -1, 75.0, 15.0},
        };

        const auto timed = [](const char* label, auto&& work) {
            const auto start = std::chrono::steady_clock::now();
            imageutils::Image result = work();
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - start).count();
            std::cout << "  " << label << "  [" << ms << " ms]" << std::endl;
            return result;
        };

        // The baseline to judge the rest against: same neighbourhood as B3, no
        // colour term at all, so every edge in the picture softens.
        const imageutils::Image gaussian =
            timed("B1 - Gaussian blur, kernel 9 (edges go too)",
                  [&] { return imageutils::blur(source, 9); });
        if (gaussian.empty() || !gaussian.show("B1 - Gaussian blur, kernel 9 (edges go too)")) {
            std::cout << "showing    = blur failed or no display available\n";
            return 1;
        }

        for (const Smooth& smooth : smooths) {
            const imageutils::Image filtered = timed(smooth.title, [&] {
                return imageutils::bilateralFilter(source, smooth.diameter,
                                                   smooth.sigmaColor, smooth.sigmaSpace);
            });
            // An empty result is the in-band failure, same as everywhere else
            // in imageutils -- callers test instead of catching.
            if (filtered.empty()) {
                std::cout << "  " << smooth.title << " -- bilateralFilter failed\n";
                return 1;
            }
            if (!filtered.show(smooth.title)) {
                std::cout << "showing    = no display available\n";
                return 1;
            }
        }

        // Single-channel input is supported too, so a mask or a grayscale
        // conversion can be smoothed without widening it back to BGR first.
        const imageutils::Image graySmoothed =
            imageutils::bilateralFilter(imageutils::toGrayscale(source), 9, 75.0, 75.0);
        std::cout << "  B6 - Grayscale source (" << graySmoothed.channels()
                  << " channel)" << std::endl;
        if (!graySmoothed.show("B6 - Grayscale source, bilateral d=9")) {
            std::cout << "showing    = no display available\n";
            return 1;
        }

        // Bad arguments stay in-band as well: an empty source yields an empty
        // result, and negative sigmas are folded onto 0 rather than rejected.
        std::cout << "  empty-src  = "
                  << (imageutils::bilateralFilter(imageutils::Image{}, 9).empty()
                          ? "empty, as expected" : "unexpected")
                  << "\n  neg-sigma  = "
                  << (imageutils::bilateralFilter(source, 5, -10.0, -10.0).empty()
                          ? "unexpected" : "clamped to 0, still filtered")
                  << std::endl;
    }
    // // The three arguments, plus the window title that names them.
    // struct Sample {
    //     const char* title;
    //     int hueShiftDegrees;
    //     double saturationScale;
    //     double valueScale;
    // };
    // const std::vector<Sample> samples{
    //     {"1 - Untouched (0, 1.0, 1.0)",         0, 1.0, 1.0},
    //     {"2 - Hue +60",                        60, 1.0, 1.0},
    //     {"3 - Hue +120, saturation x1.5",     120, 1.5, 1.0},
    //     {"4 - Hue -240 (same wheel spot)",   -240, 1.5, 1.0},
    //     {"5 - Hue +180, opposite colours",    180, 1.0, 1.0},
    //     {"6 - Desaturated (saturation x0)",     0, 0.0, 1.0},
    //     {"7 - Dimmed (value x0.5)",             0, 1.0, 0.5},
    //     {"8 - Brightened (value x1.8)",         0, 1.0, 1.8},
    // };

    // // The colour-detection trio, on a still instead of a camera feed: the
    // // original, its HSV decomposition, and the mask each range selects.
    // //
    // // colorMask() converts to HSV internally and thresholds *that*, which is
    // // the point of the exercise -- thresholding the BGR image instead would
    // // read these same numbers as blue/green/red bounds and select the wrong
    // // pixels, or none at all.
    // //
    // // Bounds are OpenCV's 8-bit HSV: H is 0-179 (real degrees halved to fit a
    // // byte), S and V are 0-255. So lowH 15 / highH 25 is about 30-50 degrees on
    // // a colour wheel -- orange -- and the S/V floors keep it to pixels that are
    // // vividly orange rather than washed out or nearly black.
    // struct MaskSample {
    //     const char* title;
    //     imageutils::HsvRange range;  // lowH, lowS, lowV, highH, highS, highV
    // };
    // const std::vector<MaskSample> maskSamples{
    //     {"11 - Mask: orange (H 15-25)",  {15, 200,  90,  25, 255, 155}},
    //     {"12 - Mask: blue (H 100-130)",  {100, 120,  60, 130, 255, 255}},
    //     {"13 - Mask: anything bright",   {0,    0, 200, 179, 255, 255}},
    // };

    // std::cout << "showing    = " << samples.size() + 3 + maskSamples.size()
    //           << " windows, any key for the next" << std::endl;
    // for (const Sample& sample : samples) {
    //     const imageutils::Image adjusted = imageutils::adjustHsv(
    //         source, sample.hueShiftDegrees, sample.saturationScale, sample.valueScale);
    //     if (adjusted.empty()) {
    //         std::cout << "  " << sample.title << " -- adjustHsv failed\n";
    //         return 1;
    //     }
    //     std::cout << "  " << sample.title << std::endl;
    //     // A headless session is an ordinary outcome here, reported in-band, so
    //     // say it once and stop rather than opening windows that cannot appear.
    //     if (!adjusted.show(sample.title)) {
    //         std::cout << "showing    = no display available\n";
    //         return 1;
    //     }
    // }

    // // Last one out: a grayscale source has no hue to rotate, so the 200 degrees
    // // does nothing and only the value scale is visible.
    // const imageutils::Image grayLifted =
    //     imageutils::adjustHsv(imageutils::toGrayscale(source), 200, 1.0, 1.3);
    // std::cout << "  9 - Grayscale source, value x1.3" << std::endl;
    // grayLifted.show("9 - Grayscale source, value x1.3");

    // std::cout << "  10 - HSV decomposition" << std::endl;
    // if (!source.showHsv("10 - HSV decomposition")) {
    //     std::cout << "showing    = no display available\n";
    //     return 1;
    // }

    // for (const MaskSample& sample : maskSamples) {
    //     // Single-channel output: white where the pixel fell inside the box,
    //     // black everywhere else. An empty result is the in-band failure, same
    //     // as everywhere else in imageutils.
    //     const imageutils::Image mask = imageutils::colorMask(source, sample.range);
    //     if (mask.empty()) {
    //         std::cout << "  " << sample.title << " -- colorMask failed\n";
    //         return 1;
    //     }
    //     std::cout << "  " << sample.title << " (" << mask.channels() << " channel)"
    //               << std::endl;
    //     if (!mask.show(sample.title)) {
    //         std::cout << "showing    = no display available\n";
    //         return 1;
    //     }
    // }

    // // Markers, drawn last because drawCircle() works in place: `source` is not
    // // the same picture after this. Image is move-only, so there is no copy to
    // // scribble on instead -- code that needs the original back reloads it.
    // const int centerX = source.width() / 2;
    // const int centerY = source.height() / 2;

    // // The tutorial line, in this API: cv::circle(frame, cv::Point(300, 300), 5,
    // // cv::Scalar(0), cv::FILLED). Color{} is black and kFilled is the default
    // // thickness, so the defaults alone reproduce it -- including the part where
    // // a black 5-pixel dot on a dark screenshot is nearly impossible to find.
    // if (!source.drawCircle(300, 300, 5)) {
    //     std::cout << "draw       = drawCircle failed\n";
    //     return 1;
    // }

    // // Colours are BGR, so red is the third component, not the first.
    // source.drawCircle(centerX, centerY, 20, imageutils::Color{0, 0, 255});

    // // Same centre, wider, and a positive thickness: a 3-pixel green ring
    // // instead of a solid disc.
    // source.drawCircle(centerX, centerY, 60, imageutils::Color{0, 255, 0}, 3);

    // // Hanging off the right edge on purpose. Clipped to what fits, and still
    // // true -- being partly off-image is not an error.
    // const bool clipped = source.drawCircle(source.width() + 40, centerY, 80,
    //                                        imageutils::Color{255, 255, 0}, 4);
    // std::cout << "draw       = off-image circle returned "
    //           << (clipped ? "true (clipped)" : "false") << '\n';

    // // A negative radius is the argument that is genuinely wrong, and it is
    // // reported in-band like everything else rather than throwing.
    // std::cout << "draw       = negative radius returned "
    //           << (source.drawCircle(centerX, centerY, -1) ? "true" : "false, as expected")
    //           << '\n';

    // std::cout << "  14 - Markers drawn in place" << std::endl;
    // if (!source.show("14 - Markers drawn in place")) {
    //     std::cout << "showing    = no display available\n";
    //     return 1;
    // }

    std::cout << "showing    = done\n";
    return 0;
}

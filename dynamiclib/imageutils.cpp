#include "imageutils.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <utility>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

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

namespace {

// LoadMode exists so the header never names OpenCV; this is where it is
// translated back into the imread flag it stands for.
int toImreadFlag(LoadMode mode) {
    switch (mode) {
    case LoadMode::Grayscale: return cv::IMREAD_GRAYSCALE;  // 0
    case LoadMode::Unchanged: return cv::IMREAD_UNCHANGED;  // -1
    case LoadMode::Color:     break;
    }
    return cv::IMREAD_COLOR;  // 1, also the answer for an out-of-range cast
}

// A BGRA image (LoadMode::Unchanged on a PNG with transparency) split into the
// colour every transform below knows how to handle and the alpha plane they
// do not. `alpha` stays empty for 1- and 3-channel input, and `bgr` is then
// the input itself, borrowed rather than copied.
void splitAlpha(const cv::Mat& input, cv::Mat& bgr, cv::Mat& alpha) {
    if (input.channels() == 4) {
        cv::cvtColor(input, bgr, cv::COLOR_BGRA2BGR);
        cv::extractChannel(input, alpha, 3);
    } else {
        bgr = input;
        alpha.release();
    }
}

// The other half of splitAlpha(): puts the alpha plane back on a 3-channel
// result so the transparency survives the edit. A no-op when there was none.
cv::Mat restoreAlpha(cv::Mat bgr, const cv::Mat& alpha) {
    if (alpha.empty()) {
        return bgr;
    }
    cv::Mat bgra;
    cv::merge(std::vector<cv::Mat>{bgr, alpha}, bgra);
    return bgra;
}

}  // namespace

bool Image::load(const std::string& path, LoadMode mode) {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
    // Reject anything that isn't an existing regular file before calling into
    // OpenCV. This keeps imgcodecs' own findDecoder warnings off the
    // consumer's stderr for the common bad-path case (missing file, a
    // directory, ...). A file that exists but is corrupt still reaches
    // cv::imread below and OpenCV may still log for that; that's accepted.
    try {
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
            impl_->mat.release();
            return false;
        }
    } catch (const std::filesystem::filesystem_error&) {
        impl_->mat.release();
        return false;
    }
    try {
        cv::Mat loaded = cv::imread(path, toImreadFlag(mode));
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

bool Image::drawCircle(int centerX, int centerY, int radius, const Color& color,
                       int thickness) {
    if (empty() || radius < 0) {
        return false;
    }
    try {
        // Scalar's components are BGR in the same order as Color's members, so
        // this is a straight copy. On a single-channel image OpenCV reads only
        // the first of them, which is why Color documents `blue` as the one
        // that counts there.
        const cv::Scalar bgr(std::clamp(color.blue, 0, 255),
                             std::clamp(color.green, 0, 255),
                             std::clamp(color.red, 0, 255),
                             // Fourth component: alpha on a BGRA image, ignored
                             // otherwise. Left at Scalar's default of 0, every
                             // circle on a transparent PNG would be invisible.
                             255);
        // Anything below kFilled would be rejected by OpenCV; fold it onto the
        // sentinel rather than failing, since "more negative" means nothing.
        const int stroke = thickness < kFilled ? kFilled : thickness;
        cv::circle(impl_->mat, cv::Point(centerX, centerY), radius, bgr, stroke);
        return true;
    } catch (const cv::Exception&) {
        return false;
    }
}

bool Image::showCameraPreview(int cameraIndex, const std::string& windowTitle) const {
    // cv::waitKey returns the raw key code; 27 is Esc.
    constexpr int kEscape = 27;
    // Long enough to pump the GUI event loop, short enough not to cap the
    // frame rate: the camera's own read() is what paces this loop.
    constexpr int kPollMs = 1;

    try {
        cv::VideoCapture capture(cameraIndex);
        if (!capture.isOpened()) {
            return false;
        }

        cv::namedWindow(windowTitle, cv::WINDOW_AUTOSIZE);

        cv::Mat frame;
        while (true) {
            capture >> frame;
            // An empty frame means the stream ended -- device unplugged, or a
            // file-backed capture ran out. Not a failure, just the end.
            if (frame.empty()) {
                break;
            }

            cv::imshow(windowTitle, frame);

            if (cv::waitKey(kPollMs) == kEscape) {
                break;
            }

            // The window's own close button has to end the loop too; otherwise
            // the next imshow would silently recreate the window and the
            // preview would be unclosable except by Esc.
            if (cv::getWindowProperty(windowTitle, cv::WND_PROP_VISIBLE) < 1) {
                break;
            }
        }

        cv::destroyWindow(windowTitle);
        return true;
    } catch (const cv::Exception&) {
        // Includes the headless case: a build without GUI support throws from
        // namedWindow/imshow rather than returning an error.
        try {
            cv::destroyWindow(windowTitle);
        } catch (const cv::Exception&) {
        }
        return false;
    }
}

bool Image::show(const std::string& windowTitle, int delayMs) const {
    // Long enough to pump the GUI event loop without spinning the CPU, short
    // enough that the close button still feels immediate. A single blocking
    // waitKey(0) would not do: it never returns when the window is closed with
    // the mouse instead of the keyboard. For the same reason a timed show is
    // still polled in slices rather than handed to one waitKey(delayMs).
    constexpr int kPollMs = 30;

    if (empty()) {
        return false;
    }

    try {
        cv::namedWindow(windowTitle, cv::WINDOW_AUTOSIZE);
        cv::imshow(windowTitle, impl_->mat);

        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);
        while (true) {
            // waitKey's own convention: 0 (or less) means no deadline at all.
            int wait = kPollMs;
            if (delayMs > 0) {
                const auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                                      deadline - std::chrono::steady_clock::now()).count();
                if (left <= 0) {
                    break;
                }
                wait = static_cast<int>(std::min<long long>(kPollMs, left));
            }
            // >= 0 is any key; -1 is the poll timing out with nothing pressed.
            if (cv::waitKey(wait) >= 0) {
                break;
            }
            if (cv::getWindowProperty(windowTitle, cv::WND_PROP_VISIBLE) < 1) {
                break;
            }
        }

        cv::destroyWindow(windowTitle);
        return true;
    } catch (const cv::Exception&) {
        // Includes the headless case: a build without GUI support throws from
        // namedWindow/imshow rather than returning an error.
        try {
            cv::destroyWindow(windowTitle);
        } catch (const cv::Exception&) {
        }
        return false;
    }
}

bool Image::showHsv(const std::string& windowTitle) const {
    // The conversion is the only new work here; the window handling, the
    // close-button polling and the headless case are all show()'s already.
    const Image hsv = toHsv(*this);
    if (hsv.empty()) {
        return false;
    }
    return hsv.show(windowTitle);
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

bool playVideo(const std::string& path, const std::string& windowTitle) {
    constexpr int kEscape = 27;
    constexpr int kSpace = 32;
    // While paused there is no frame to pace against, so just pump the GUI
    // event loop often enough that Space and the close button stay responsive.
    constexpr int kPausedPollMs = 30;
    // What to fall back to when the container declares no usable frame rate --
    // some webcam captures and a few odd files report 0 or NaN.
    constexpr double kFallbackFps = 25.0;

    // Keep videoio's own backend warnings off the consumer's stderr for the
    // common bad-path case, exactly as Image::load does for imgcodecs.
    try {
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
            return false;
        }
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }

    try {
        cv::VideoCapture capture(path);
        if (!capture.isOpened()) {
            return false;
        }

        const double fps = capture.get(cv::CAP_PROP_FPS);
        // NaN fails both comparisons, so this catches it along with 0 and the
        // nonsense values some containers carry.
        const double usableFps = (fps > 0.0 && fps < 1000.0) ? fps : kFallbackFps;
        // At least 1ms: waitKey(0) would block forever instead of advancing.
        const int frameDelayMs = std::max(1, static_cast<int>(1000.0 / usableFps));

        cv::namedWindow(windowTitle, cv::WINDOW_AUTOSIZE);

        cv::Mat frame;
        bool stopped = false;
        while (!stopped) {
            capture >> frame;
            // An empty frame is the end of the video, not a failure.
            if (frame.empty()) {
                break;
            }
            // cv::circle(frame, cv::Point(frame.cols / 2, frame.rows / 2), 50, cv::Scalar(0, 0, 255), 3);
            cv::imshow(windowTitle, frame);

            int key = cv::waitKey(frameDelayMs);
            if (key == kSpace) {
                // Hold on the frame already on screen. Re-showing it is not
                // needed; only the event loop has to keep running.
                while (true) {
                    key = cv::waitKey(kPausedPollMs);
                    if (key == kSpace || key == kEscape) {
                        break;
                    }
                    if (cv::getWindowProperty(windowTitle, cv::WND_PROP_VISIBLE) < 1) {
                        stopped = true;
                        break;
                    }
                }
            }

            if (key == kEscape) {
                break;
            }

            // The window's own close button has to end playback too; otherwise
            // the next imshow would silently recreate the window and the video
            // would be unstoppable except by Esc.
            if (!stopped && cv::getWindowProperty(windowTitle, cv::WND_PROP_VISIBLE) < 1) {
                break;
            }
        }

        cv::destroyWindow(windowTitle);
        return true;
    } catch (const cv::Exception&) {
        // Includes the headless case: a build without GUI support throws from
        // namedWindow/imshow rather than returning an error.
        try {
            cv::destroyWindow(windowTitle);
        } catch (const cv::Exception&) {
        }
        return false;
    }
}

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

Image toHsv(const Image& src) {
    if (src.empty()) {
        return Image{};
    }
    try {
        const cv::Mat& input = detail::Access::mat(src);
        cv::Mat output;
        if (input.channels() == 1) {
            // cvtColor has no GRAY2HSV; go through BGR, which yields hue 0 and
            // saturation 0 with the original intensity as value.
            cv::Mat bgr;
            cv::cvtColor(input, bgr, cv::COLOR_GRAY2BGR);
            cv::cvtColor(bgr, output, cv::COLOR_BGR2HSV);
        } else {
            cv::cvtColor(input, output, cv::COLOR_BGR2HSV);
        }
        return detail::Access::wrap(std::move(output));
    } catch (const cv::Exception&) {
        return Image{};
    }
}

Image adjustHsv(const Image& src, int hueShiftDegrees, double saturationScale,
                double valueScale) {
    if (src.empty()) {
        return Image{};
    }
    try {
        const cv::Mat& input = detail::Access::mat(src);
        // Alpha is set aside rather than run through HSV, which has no slot
        // for it, and reattached at the end -- a hue shift is not a reason to
        // lose the transparency.
        cv::Mat bgr;
        cv::Mat alpha;
        if (input.channels() == 1) {
            cv::cvtColor(input, bgr, cv::COLOR_GRAY2BGR);
        } else {
            splitAlpha(input, bgr, alpha);
        }

        cv::Mat hsv;
        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

        // OpenCV packs a 0-360 hue into a byte by halving it, so a degree of
        // rotation is half a step. Wrap first, then halve, so that a shift of
        // 359 does not turn into 179.5 truncated twice over.
        const int wrapped = ((hueShiftDegrees % 360) + 360) % 360;
        const int hueOffset = wrapped / 2;
        const double saturation = std::max(saturationScale, 0.0);
        const double value = std::max(valueScale, 0.0);

        for (int y = 0; y < hsv.rows; ++y) {
            auto* row = hsv.ptr<cv::Vec3b>(y);
            for (int x = 0; x < hsv.cols; ++x) {
                cv::Vec3b& pixel = row[x];
                // Hue wraps around the wheel; saturation and value clamp.
                pixel[0] = static_cast<unsigned char>((pixel[0] + hueOffset) % 180);
                pixel[1] = cv::saturate_cast<unsigned char>(pixel[1] * saturation);
                pixel[2] = cv::saturate_cast<unsigned char>(pixel[2] * value);
            }
        }

        cv::Mat output;
        cv::cvtColor(hsv, output, cv::COLOR_HSV2BGR);
        return detail::Access::wrap(restoreAlpha(std::move(output), alpha));
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

Image bilateralFilter(const Image& src, int diameter, double sigmaColor,
                      double sigmaSpace) {
    if (src.empty()) {
        return Image{};
    }
    try {
        // Negative sigmas are meaningless to the weighting and OpenCV does not
        // reject them, so fold them onto 0 rather than letting them produce
        // something nobody can reason about.
        const double colorSigma = std::max(sigmaColor, 0.0);
        const double spaceSigma = std::max(sigmaSpace, 0.0);
        // Anything <= 0 means "derive the neighbourhood from sigmaSpace", which
        // is OpenCV's own convention; pass it through unchanged so the two
        // spellings of that request stay one code path.
        const int d = diameter > 0 ? diameter : -1;

        // bilateralFilter refuses to work in place, so this must be a distinct
        // Mat -- it cannot be the borrowed input aliased.
        //
        // The filter itself only takes 1 or 3 channels, so a BGRA source is
        // smoothed on its colour alone and gets its alpha back untouched --
        // smoothing a transparency edge would make it a soft halo.
        cv::Mat bgr;
        cv::Mat alpha;
        splitAlpha(detail::Access::mat(src), bgr, alpha);
        cv::Mat output;
        cv::bilateralFilter(bgr, output, d, colorSigma, spaceSigma);
        return detail::Access::wrap(restoreAlpha(std::move(output), alpha));
    } catch (const cv::Exception&) {
        // 8-bit 1-, 3- and 4-channel input is all this supports; anything
        // else lands here, in-band with every other failure in this file.
        return Image{};
    }
}

Image colorMask(const Image& src, const HsvRange& range) {
    if (src.empty()) {
        return Image{};
    }
    try {
        cv::Mat bgr;
        cv::Mat alpha;
        splitAlpha(detail::Access::mat(src), bgr, alpha);

        cv::Mat hsv;
        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

        cv::Mat mask;
        cv::inRange(hsv,
                    cv::Scalar(range.lowH, range.lowS, range.lowV),
                    cv::Scalar(range.highH, range.highS, range.highV),
                    mask);
        // A fully transparent pixel still carries a colour, usually whatever
        // the editor left there, but nobody can see it -- so it is never a
        // match, however well that hidden colour fits the range.
        if (!alpha.empty()) {
            mask.setTo(0, alpha == 0);
        }
        return detail::Access::wrap(std::move(mask));
    } catch (const cv::Exception&) {
        return Image{};
    }
}

std::string openCvVersion() {
    try {
        return cv::getVersionString();
    } catch (const cv::Exception&) {
        return "";
    }
}

}  // namespace imageutils

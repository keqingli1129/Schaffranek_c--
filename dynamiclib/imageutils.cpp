#include "imageutils.h"

#include <algorithm>
#include <filesystem>
#include <utility>

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

bool Image::load(const std::string& path) {
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

std::string openCvVersion() {
    try {
        return cv::getVersionString();
    } catch (const cv::Exception&) {
        return "";
    }
}

}  // namespace imageutils

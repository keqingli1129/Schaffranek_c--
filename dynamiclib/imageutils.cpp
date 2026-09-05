#include "imageutils.h"

#include <filesystem>

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
    try {
        return cv::getVersionString();
    } catch (const cv::Exception&) {
        return "";
    }
}

}  // namespace imageutils

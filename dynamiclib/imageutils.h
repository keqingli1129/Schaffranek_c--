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

    // Opens a camera and shows its frames in a window, one after another, until
    // Esc is pressed or the window is closed. Blocks for as long as the preview
    // runs, so this is a foreground call, not a background stream.
    //
    // Displays only: this Image is neither read nor written, hence const. It
    // captures nothing, because the frame on screen when the window closes is
    // whichever one happened to be there, not one anybody chose.
    //
    // Returns false when the camera cannot be opened -- no device, in use by
    // something else, or no display to draw into. That is an ordinary outcome
    // here, reported in-band like every other failure in this header, so
    // callers check the bool rather than catching.
    bool showCameraPreview(int cameraIndex = 0,
                           const std::string& windowTitle = "Original") const;

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

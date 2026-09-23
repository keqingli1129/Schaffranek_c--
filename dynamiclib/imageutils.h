#pragma once

#include <memory>
#include <string>

#include "imageutils_export.h"

namespace imageutils {

namespace detail {
struct Access;
}  // namespace detail

// A colour to draw with: BGR components in OpenCV's channel order, 0-255.
// Values outside that range are clamped rather than rejected.
//
// Defaults to black, which is worth knowing before using it: a black marker on
// a dark image is drawn and invisible, not skipped. On a single-channel image
// (a mask, or a grayscale conversion) only `blue` is used, because it is the
// first component -- so Color{255, 0, 0} is white there, not blue.
struct Color {
    int blue = 0;
    int green = 0;
    int red = 0;
};

// Thickness sentinel: fill the shape rather than stroking its outline. Any
// negative thickness means the same thing; this is the readable spelling.
inline constexpr int kFilled = -1;

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

    // Draws a circle into this image, in place -- the only mutating operation
    // here besides load(), and the reason it is not const.
    //
    // centerX and centerY are pixel coordinates in (x, y) order: x is the
    // column, y is the row. That is OpenCV's drawing convention and the
    // transpose of the (row, column) order its pixel accessors use, so the two
    // are easy to swap by accident.
    //
    // radius is in pixels. thickness is the outline width, or kFilled for a
    // solid disc; values below kFilled are treated as kFilled.
    //
    // A circle whose centre lies outside the image, or which runs off the
    // edge, is clipped silently and still returns true -- nothing was wrong
    // with the request, there was just less of it on screen.
    //
    // Returns false when the image is empty, when radius is negative, and when
    // OpenCV rejects the request. In-band like every other failure here.
    bool drawCircle(int centerX, int centerY, int radius,
                    const Color& color = Color{}, int thickness = kFilled);

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

    // Shows this image in a window and blocks until the viewer dismisses it --
    // any key, or the window's close button. Nothing is modified, hence const.
    //
    // Returns false when the image is empty, and when there is no display to
    // draw into (a headless session, or an OpenCV built without GUI support).
    // Both are ordinary outcomes, reported in-band like every other failure in
    // this header, so callers check the bool rather than catching.
    bool show(const std::string& windowTitle = "Image") const;

    // Converts to HSV and shows the result in a window, blocking exactly like
    // show(). What appears on screen is the HSV data drawn as if it were BGR --
    // hue in the blue channel, saturation in green, value in red -- which is
    // the usual way of eyeballing an HSV decomposition, not a second rendering
    // of the original colours.
    //
    // Returns false when the image is empty, when the conversion fails, and
    // when there is no display to draw into.
    bool showHsv(const std::string& windowTitle = "HSV") const;

    bool empty() const;
    int width() const;     // 0 when empty
    int height() const;    // 0 when empty
    int channels() const;  // 0 when empty

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend struct detail::Access;
};

// Plays a video file in a window, frame after frame, paced by the frame rate
// the file declares. Blocks until the video ends, Esc is pressed or the window
// is closed; Space pauses and resumes. A foreground call, exactly like
// Image::showCameraPreview() -- not a background stream.
//
// A viewer, not a loader: frames go straight to the window and none of them is
// decoded into an Image, because the frame on screen when playback stops is
// whichever one happened to be there, not one anybody chose.
//
// Returns false when the file is missing or cannot be opened -- wrong path, or
// a container or codec this OpenCV build has no decoder for -- and when there
// is no display to draw into. Ordinary outcomes, reported in-band like every
// other failure in this header, so callers check the bool rather than catching.
IMAGEUTILS_API bool playVideo(const std::string& path,
                              const std::string& windowTitle = "Video");

// Synthesized 3-channel BGR pattern: blue ramps with x, green ramps with y, red
// is a 16-pixel checkerboard. Deterministic, so tests can assert on it. Exists so
// the demo and the tests need no image files on disk. Empty Image if width or
// height <= 0.
IMAGEUTILS_API Image makeTestPattern(int width, int height);

// Single-channel copy of src. Already-grayscale input is passed through.
IMAGEUTILS_API Image toGrayscale(const Image& src);

// 3-channel HSV copy of src, with the OpenCV 8-bit ranges: H in [0, 180),
// S and V in [0, 255]. Single-channel input is treated as grayscale BGR first,
// so it converts to a zero-hue, zero-saturation image rather than failing.
IMAGEUTILS_API Image toHsv(const Image& src);

// HSV-space edit, returned as an ordinary 3-channel BGR image -- unlike
// toHsv(), the result is meant to be looked at, saved and shown directly.
//
// hueShiftDegrees rotates the hue wheel and wraps, so -30 and 330 do the same
// thing; it is given in real degrees (0-360), not OpenCV's halved 0-179.
// saturationScale and valueScale multiply those channels and are clamped at 0;
// values above 1 saturate at 255 rather than wrapping. Passing 0, 1.0, 1.0
// gives a copy of src. Single-channel input is treated as grayscale BGR first.
//
// Empty Image if src is empty.
IMAGEUTILS_API Image adjustHsv(const Image& src, int hueShiftDegrees,
                               double saturationScale = 1.0, double valueScale = 1.0);

// Resized copy. Returns an empty Image if width or height <= 0.
IMAGEUTILS_API Image resize(const Image& src, int width, int height);

// Gaussian blur. kernelSize is forced odd and >= 1; even values are incremented.
IMAGEUTILS_API Image blur(const Image& src, int kernelSize);
// Inclusive HSV bounds, in OpenCV's 8-bit convention: H is 0-179, S and V are
// 0-255. See the note on hue halving if you're translating from a 0-360 H
// or a color picker's 0-100% S/V.
struct HsvRange {
    int lowH = 0,  lowS = 0,   lowV = 0;
    int highH = 179, highS = 255, highV = 255;
};

IMAGEUTILS_API Image colorMask(const Image& src, const HsvRange& range);

// Runtime OpenCV version string, e.g. "4.6.0".
IMAGEUTILS_API std::string openCvVersion();

}  // namespace imageutils

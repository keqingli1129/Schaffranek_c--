#include <filesystem>
#include <iostream>
#include <string>
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
    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    std::cout << mathutils::greet(name) << std::endl;

    const std::vector<int> values{3, 7, 11, 12, 42};

    std::cout << "sum     = " << mathutils::sum(values) << '\n';
    std::cout << "average = " << mathutils::average(values) << '\n';
    std::cout << "5!      = " << mathutils::factorial(5) << '\n';

    std::cout << "primes  =";
    for (int value : values) {
        if (mathutils::isPrime(value)) {
            std::cout << ' ' << value;
        }
    }
    std::cout << std::endl;

    // 2x + y = 5, x - 3y = -8  ->  x = 1, y = 3. Eigen does the work inside
    // mathutils; nothing about it reaches this translation unit.
    const std::vector<double> solution =
        mathutils::solveLinearSystem({{2.0, 1.0}, {1.0, -3.0}}, {5.0, -8.0});
    std::cout << "solved  =";
    for (double value : solution) {
        std::cout << ' ' << value;
    }
    std::cout << (solution.empty() ? " <no solution>" : "") << std::endl;

    const std::string phrase = "  A man, a plan, a canal: Panama  ";
    const std::string trimmed = stringutils::trim(phrase);

    std::cout << "trimmed    = [" << trimmed << "]\n";
    std::cout << "upper      = " << stringutils::toUpper(trimmed) << '\n';
    std::cout << "lower      = " << stringutils::toLower(trimmed) << '\n';
    std::cout << "reversed   = " << stringutils::reverse(trimmed) << '\n';
    std::cout << "palindrome = " << (stringutils::isPalindrome(trimmed) ? "yes" : "no") << '\n';

    const std::vector<std::string> words = stringutils::split("alpha,beta,gamma", ',');
    std::cout << "joined     = " << stringutils::join(words, " | ") << std::endl;

    std::cout << "opencv     = " << imageutils::openCvVersion() << '\n';

    // Failures in imageutils are in-band: a transform returns an empty Image
    // rather than throwing. Consumers check, they don't catch.
    const auto report = [](const char* label, const imageutils::Image& image) {
        std::cout << label << " = " << image.width() << 'x' << image.height()
                  << ", " << image.channels() << " channels\n";
        return !image.empty();
    };

    const imageutils::Image pattern = imageutils::makeTestPattern(256, 128);
    if (!report("pattern   ", pattern)) { return 1; }

    // Chained in memory -- no disk round trip between steps.
    const imageutils::Image gray = imageutils::toGrayscale(pattern);
    if (!report("grayscale ", gray)) { return 1; }

    const imageutils::Image resized = imageutils::resize(gray, 64, 32);
    if (!report("resized   ", resized)) { return 1; }

    const imageutils::Image blurred = imageutils::blur(resized, 5);
    if (!report("blurred   ", blurred)) { return 1; }

    const std::filesystem::path project = projectDir();
    const std::string outputPath = (project / "imageutils_demo.png").string();
    const bool saved = blurred.save(outputPath);
    std::cout << "saved      = " << outputPath << " (" << (saved ? "yes" : "no") << ")"
              << std::endl;

    // Opt-in: both viewers block until dismissed and need a display, so they
    // must never run in a headless build or a test harness. "--show" takes an
    // optional path; without one it falls back to the image in the source tree.
    bool wantCamera = false;
    bool wantShow = false;
    std::string showPath = (project / "Screenshot.png").string();
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--camera") {
            wantCamera = true;
        } else if (arg == "--show") {
            wantShow = true;
            // A following argument that is not itself a flag is the path.
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                showPath = argv[++i];
            }
        }
    }

    if (wantShow) {
        imageutils::Image picture;
        if (!picture.load(showPath)) {
            std::cout << "show       = cannot load " << showPath << '\n';
            return 1;
        }
        if (!report("show      ", picture)) { return 1; }
        std::cout << "show       = " << showPath << " (any key to close)" << std::endl;
        if (!picture.show(showPath)) {
            std::cout << "show       = no display\n";
            return 1;
        }
        std::cout << "show       = closed\n";
    }

    if (wantCamera) {
        std::cout << "camera     = opening (Esc to close)" << std::endl;
        // The preview displays only, so any Image will do as the receiver.
        const imageutils::Image live;
        if (!live.showCameraPreview(0, "Original")) {
            std::cout << "camera     = unavailable\n";
            return 1;
        }
        std::cout << "camera     = closed\n";
    }

    return saved ? 0 : 1;
}

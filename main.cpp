#include <iostream>
#include <string>
#include <vector>

#include "mathutils.h"
#include "stringutils.h"
#include "imageutils.h"

int main() {
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

    const std::string outputPath = "imageutils_demo.png";
    const bool saved = blurred.save(outputPath);
    std::cout << "saved      = " << outputPath << " (" << (saved ? "yes" : "no") << ")"
              << std::endl;

    return saved ? 0 : 1;
}

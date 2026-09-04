#include "stringutils.h"

#include <algorithm>
#include <cctype>

namespace stringutils {
namespace {

unsigned char asByte(char c) {
    return static_cast<unsigned char>(c);
}

}  // namespace

std::string toUpper(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](char c) { return static_cast<char>(std::toupper(asByte(c))); });
    return result;
}

std::string toLower(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](char c) { return static_cast<char>(std::tolower(asByte(c))); });
    return result;
}

std::string reverse(const std::string& text) {
    return std::string(text.rbegin(), text.rend());
}

std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(" \t\n\r\f\v");
    return text.substr(first, last - first + 1);
}

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    std::string::size_type start = 0;
    while (true) {
        const auto pos = text.find(delimiter, start);
        if (pos == std::string::npos) {
            parts.push_back(text.substr(start));
            break;
        }
        parts.push_back(text.substr(start, pos - start));
        start = pos + 1;
    }
    return parts;
}

std::string join(const std::vector<std::string>& parts, const std::string& separator) {
    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            result += separator;
        }
        result += parts[i];
    }
    return result;
}

bool isPalindrome(const std::string& text) {
    std::string cleaned;
    cleaned.reserve(text.size());
    for (char c : text) {
        if (std::isalnum(asByte(c))) {
            cleaned += static_cast<char>(std::tolower(asByte(c)));
        }
    }
    return std::equal(cleaned.begin(), cleaned.begin() + cleaned.size() / 2, cleaned.rbegin());
}

}  // namespace stringutils

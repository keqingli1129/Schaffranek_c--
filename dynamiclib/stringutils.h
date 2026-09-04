#pragma once

#include <string>
#include <vector>

#include "stringutils_export.h"

namespace stringutils {

STRINGUTILS_API std::string toUpper(const std::string& text);

STRINGUTILS_API std::string toLower(const std::string& text);

STRINGUTILS_API std::string reverse(const std::string& text);

// Drops leading and trailing whitespace.
STRINGUTILS_API std::string trim(const std::string& text);

// Splits on a single delimiter; empty fields are kept.
STRINGUTILS_API std::vector<std::string> split(const std::string& text, char delimiter);

STRINGUTILS_API std::string join(const std::vector<std::string>& parts, const std::string& separator);

// Case-insensitive palindrome check that ignores non-alphanumeric characters.
STRINGUTILS_API bool isPalindrome(const std::string& text);

}  // namespace stringutils

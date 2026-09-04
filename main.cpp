#include <iostream>
#include <string>
#include <vector>

#include "mathutils.h"
#include "stringutils.h"

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

    return 0;
}

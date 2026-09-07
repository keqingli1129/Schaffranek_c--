#include "mathutils.h"

#include <numeric>

namespace mathutils {

long long sum(const std::vector<int>& values) {
    return std::accumulate(values.begin(), values.end(), 0LL);
}

double average(const std::vector<int>& values) {
    if (values.empty()) {
        return 0.0;
    }
    return static_cast<double>(sum(values)) / static_cast<double>(values.size());
}

long long factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

bool isPrime(int n) {
    if (n < 2) {
        return false;
    }
    for (int i = 2; static_cast<long long>(i) * i <= n; ++i) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

std::string greet(const std::string& name) {
    return "Hello, " + name + "!";
}

}  // namespace mathutils

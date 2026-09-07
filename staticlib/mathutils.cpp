#include "mathutils.h"

#include <algorithm>
#include <cstddef>
#include <numeric>

#include <Eigen/Dense>

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

std::vector<double> solveLinearSystem(const std::vector<std::vector<double>>& a,
                                      const std::vector<double>& b) {
    const std::size_t n = b.size();
    if (n == 0 || a.size() != n) {
        return {};
    }
    if (std::any_of(a.begin(), a.end(),
                    [n](const std::vector<double>& row) { return row.size() != n; })) {
        return {};
    }

    const auto extent = static_cast<Eigen::Index>(n);
    Eigen::MatrixXd matrix(extent, extent);
    Eigen::VectorXd rhs(extent);
    for (std::size_t i = 0; i < n; ++i) {
        const auto row = static_cast<Eigen::Index>(i);
        rhs(row) = b[i];
        for (std::size_t j = 0; j < n; ++j) {
            matrix(row, static_cast<Eigen::Index>(j)) = a[i][j];
        }
    }

    // partialPivLu is the fast path, but it reports nothing about conditioning:
    // on a singular matrix it returns inf/NaN, or a vector that simply does not
    // satisfy the system. Substituting the answer back is the cheap check that
    // catches both. Scaling by ||b|| keeps the tolerance meaningful whether the
    // right-hand side is near zero or very large.
    const Eigen::VectorXd x = matrix.partialPivLu().solve(rhs);
    const double tolerance = 1e-9 * std::max(1.0, rhs.norm());
    if (!x.allFinite() || (matrix * x - rhs).norm() > tolerance) {
        return {};
    }

    return {x.data(), x.data() + n};
}

std::string greet(const std::string& name) {
    return "Hello, " + name + "!";
}

}  // namespace mathutils

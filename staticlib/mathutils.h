#pragma once

#include <string>
#include <vector>

namespace mathutils {

// Sum of all elements (0 for an empty vector).
long long sum(const std::vector<int>& values);

// Arithmetic mean; returns 0.0 for an empty vector.
double average(const std::vector<int>& values);

// n! computed iteratively. Negative input returns 1.
long long factorial(int n);

bool isPrime(int n);

// Solves the dense linear system a*x = b, where `a` is row-major and square.
// Backed by Eigen's LU decomposition with partial pivoting.
//
// Failure is in-band: returns an empty vector when `a` is not square, when it
// is ragged, when its size does not match `b`, or when no solution satisfies
// the system. Callers check, they don't catch.
//
// A singular but consistent system has infinitely many solutions; this returns
// one of them rather than an empty vector, because the answer it hands back
// does solve the system. Callers needing uniqueness must check the rank.
std::vector<double> solveLinearSystem(const std::vector<std::vector<double>>& a,
                                      const std::vector<double>& b);

// Greeting used by the demo in main.cpp.
std::string greet(const std::string& name);

}  // namespace mathutils

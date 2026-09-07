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

// Greeting used by the demo in main.cpp.
std::string greet(const std::string& name);

}  // namespace mathutils

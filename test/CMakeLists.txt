# The root CMakeLists.txt calls enable_testing() after add_subdirectory(test),
# so without this local call ctest silently registers zero tests here even
# though add_test() below runs without error.
enable_testing()

add_executable(imageutils_test imageutils_test.cpp)

# Links imageutils ONLY. If OpenCV ever leaks into the public header, this
# target stops compiling -- that is the point.
target_link_libraries(imageutils_test PRIVATE imageutils)

add_test(NAME imageutils_test COMMAND imageutils_test)

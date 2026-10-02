add_executable(imageutils_test imageutils_test.cpp)

# Links imageutils ONLY. If OpenCV ever leaks into the public header, this
# target stops compiling -- that is the point.
target_link_libraries(imageutils_test PRIVATE imageutils)

add_test(NAME imageutils_test COMMAND imageutils_test)

# A real BGRA file for the 4-channel checks; the test runs from build/test, so
# the path is baked in rather than resolved at runtime.
target_compile_definitions(imageutils_test
    PRIVATE IMAGEUTILS_TEST_BGRA_PATH="${PROJECT_SOURCE_DIR}/Screenshot.png")

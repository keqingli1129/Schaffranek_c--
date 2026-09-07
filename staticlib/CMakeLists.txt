add_library(mathutils STATIC mathutils.cpp)

target_include_directories(mathutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(mathutils PUBLIC cxx_std_23)

include(FetchContent)

# Eigen's canonical home is GitLab; the GitHub repository is a read-only mirror.
#
# SOURCE_SUBDIR deliberately names a directory that holds no CMakeLists.txt, so
# FetchContent downloads the headers WITHOUT configuring Eigen as a subproject.
# Eigen is header-only, so nothing is lost -- and its own CMakeLists would
# otherwise force BUILD_TESTING OFF, which the root CMakeLists reads to decide
# whether to add test/. Configuring Eigen normally silently deletes our tests.
FetchContent_Declare(
    Eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4.0
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR  no-cmake-build
)
FetchContent_MakeAvailable(Eigen)

# The include path is the entire dependency. SYSTEM keeps Eigen's own headers
# from producing warnings in builds that turn them on.
add_library(eigen INTERFACE)
add_library(Eigen3::Eigen ALIAS eigen)
target_include_directories(eigen SYSTEM INTERFACE ${eigen_SOURCE_DIR})

# PRIVATE keeps Eigen's headers off every consumer of mathutils -- the same
# discipline imageutils applies to OpenCV. mathutils.h stays plain std types.
target_link_libraries(mathutils PRIVATE Eigen3::Eigen)

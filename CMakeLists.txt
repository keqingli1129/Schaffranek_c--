cmake_minimum_required(VERSION 3.22.0)

# Emit build/compile_commands.json for IDE/IntelliSense and clang tooling.
# Must be set before project(), which is when the languages are enabled.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Schaffranek_c++ VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(staticlib)
add_subdirectory(dynamiclib)

add_executable(Schaffranek_c++ main.cpp)
target_link_libraries(Schaffranek_c++ PRIVATE mathutils stringutils imageutils)

include(CTest)          # defines BUILD_TESTING and calls enable_testing() when ON

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

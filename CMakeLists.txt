cmake_minimum_required(VERSION 3.22.0)
project(Schaffranek_c++ VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(staticlab)
add_subdirectory(dynamiclib)

add_executable(Schaffranek_c++ main.cpp)
target_link_libraries(Schaffranek_c++ PRIVATE mathutils stringutils)

include(CTest)
enable_testing()

add_subdirectory(test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

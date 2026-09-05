add_library(stringutils SHARED stringutils.cpp)

target_include_directories(stringutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(stringutils PUBLIC cxx_std_23)

# Export only what is marked STRINGUTILS_API.
set_target_properties(stringutils PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

# Selects dllexport over dllimport while building the library itself.
target_compile_definitions(stringutils PRIVATE STRINGUTILS_BUILDING)

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_library(imageutils SHARED imageutils.cpp)

target_include_directories(imageutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(imageutils PUBLIC cxx_std_23)

# PRIVATE keeps OpenCV's headers and libraries off every consumer of imageutils.
target_link_libraries(imageutils PRIVATE
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
)

# Export only what is marked IMAGEUTILS_API.
set_target_properties(imageutils PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

# Selects dllexport over dllimport while building the library itself.
target_compile_definitions(imageutils PRIVATE IMAGEUTILS_BUILDING)

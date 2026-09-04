add_library(stringutils SHARED stringutils.cpp)

target_include_directories(stringutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(stringutils PUBLIC cxx_std_17)

# Export only what is marked STRINGUTILS_API.
set_target_properties(stringutils PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

# Selects dllexport over dllimport while building the library itself.
target_compile_definitions(stringutils PRIVATE STRINGUTILS_BUILDING)

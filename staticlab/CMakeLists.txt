add_library(mathutils STATIC mathutils.cpp)

target_include_directories(mathutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(mathutils PUBLIC cxx_std_17)

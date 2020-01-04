
cmake_minimum_required(VERSION 3.3)

project(libflow CXX)

add_library(flow INTERFACE)
target_sources(flow INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flow.hpp

)

target_compile_features(flow INTERFACE cxx_std_17)
target_include_directories(flow INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_options(flow INTERFACE /permissive-)
endif()

set(CMAKE_CXX_EXTENSIONS Off)

add_subdirectory(example)
add_subdirectory(test)
add_subdirectory(tools)

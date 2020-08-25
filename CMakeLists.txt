
cmake_minimum_required(VERSION 3.3)

project(libflow CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

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

option(FLOW_BUILD_DOCS "Build Doxygen/Sphinx/Breathe documentation" Off)
option(FLOW_BUILD_EXAMPLES "Build libflow examples" On)
option(FLOW_BUILD_TESTS "Build libflow tests" On)
option(FLOW_BUILD_TOOLS "Build single-header generator tool" Off)

if (${FLOW_BUILD_DOCS})
    add_subdirectory(doc)
endif()

if (${FLOW_BUILD_EXAMPLES})
    add_subdirectory(example)
endif()

if (${FLOW_BUILD_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()

if (${FLOW_BUILD_TOOLS})
    add_subdirectory(tools)
endif()

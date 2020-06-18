
add_executable(example_random_ints random_ints.cpp)
target_link_libraries(example_random_ints flow)

add_executable(example_pythagorean_triples pythagorean_triples.cpp)
target_link_libraries(example_pythagorean_triples flow)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(example_pythagorean_triples PRIVATE -fcoroutines-ts)
endif()

if (MSVC)
    target_compile_options(example_pythagorean_triples PRIVATE /await)
endif()
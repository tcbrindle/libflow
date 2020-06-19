
function(add_example NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE flow)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${NAME} PRIVATE -fcoroutines-ts)
    endif()

    if (MSVC)
        target_compile_options(${name} PRIVATE /await)
    endif()
endfunction()

add_example(example_fibonacci fibonacci.cpp)
add_example(example_fibonacci_coro fibonacci_coro.cpp)
add_example(example_pythagorean_triples pythagorean_triples.cpp)
add_example(example_random_ints random_ints.cpp)

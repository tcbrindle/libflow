
function(add_example NAME)
    add_executable(example_${NAME} ${NAME}.cpp)
    target_link_libraries(example_${NAME} PRIVATE flow)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(example_${NAME} PRIVATE -fcoroutines-ts)
    endif()

endfunction()

add_example(custom_adaptor)
add_example(custom_adaptor2)
add_example(fibonacci)
add_example(fibonacci_coro)
add_example(leetcode168)
add_example(leetcode176)
add_example(leetcode210)
add_example(leetcode1437)
add_example(leetcode1672)
add_example(pythagorean_triples)
add_example(random_ints)

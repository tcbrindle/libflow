
add_library(catch-main catch_main.cpp)
target_compile_features(catch-main PUBLIC cxx_std_17)

add_executable(test-libflow
    test_invoke.cpp
    test_maybe.cpp
    test_predicates.cpp
    test_basic.cpp
    test_subflow.cpp

    # Sources
    test_any_flow.cpp
    test_async.cpp
    test_c_str.cpp
    test_empty.cpp
    test_iota.cpp
    test_from.cpp
    test_from_istream.cpp
    test_from_istreambuf.cpp
    test_of.cpp

    # Operations
    test_all_any_none.cpp
    test_cartesian_product.cpp
    test_cartesian_product_with.cpp
    test_chain.cpp
    test_chunk.cpp
    test_collect.cpp
    test_contains.cpp
    test_count.cpp
    test_count_if.cpp
    test_cycle.cpp
    test_deref.cpp
    test_drop.cpp
    test_drop_while.cpp
    test_equal.cpp
    test_filter.cpp
    test_find.cpp
    test_flatten.cpp
    test_for_each.cpp
    test_group_by.cpp
    test_inspect.cpp
    test_interleave.cpp
    test_is_sorted.cpp
    test_map.cpp
    test_map_refinements.cpp
    test_minmax.cpp
    test_output_to.cpp
    test_product.cpp
    test_reverse.cpp
    test_slide.cpp
    test_split.cpp
    test_stride.cpp
    test_sum.cpp
    test_take.cpp
    test_take_while.cpp
    test_to.cpp
    test_write_to.cpp
    test_zip.cpp
    test_zip_with.cpp
)
target_link_libraries(test-libflow flow catch-main)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    target_precompile_headers(test-libflow PRIVATE "catch.hpp")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(test-libflow PRIVATE -Wall -Wextra -pedantic
        -ftemplate-backtrace-limit=0)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(test-libflow PRIVATE -Wall -Wextra -pedantic
        -Wno-missing-braces
        -Wno-c++98-compat
        -fcoroutines-ts
        -ftemplate-backtrace-limit=0)
endif()

include(Catch)
catch_discover_tests(test-libflow)


add_executable(test-libflow
    catch_main.cpp
    test_maybe.cpp
    test_basic.cpp

    # Sources
    test_async.cpp
    test_empty.cpp
    test_from.cpp
    test_from_istream.cpp
    test_of.cpp

    # Operations
    test_collect.cpp
    test_flatten.cpp
    test_to.cpp


    test_quick.cpp
)
target_link_libraries(test-libflow flow)

if (MSVC)
    target_compile_options(test-libflow PUBLIC /await)
endif()

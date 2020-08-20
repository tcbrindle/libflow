
#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <string_view>

namespace {

constexpr bool test_zip3()
{
    std::array arr{1, 2, 3};

    auto f = flow::zip(arr, flow::c_str("zipzipzip"), flow::iota(100.0));

    auto [a, b, c] = f.next().value();

    if (std::addressof(a) != arr.data()) {
        return false;
    }

    if (b != 'z') {
        return false;
    }

    if (c != 100.0) {
        return false;
    }

    return true;
}
#if !COMPILER_IS_MSVC
static_assert(test_zip3());
#endif

constexpr bool test_zip2()
{
    std::array arr{1.0, 99.0, 1.77e33};

    auto f = flow::zip(arr, flow::iota('a', 'z'));

    if (f.size() != 3) {
        return false;
    }

    if (f.subflow().count() != 3) {
        return false;
    }

    auto [a, b] = f.next().value();

    if (std::addressof(a) != arr.data()) {
        return false;
    }

    if (b != 'a') {
        return false;
    }

    return true;
}
#if !COMPILER_IS_MSVC
static_assert(test_zip2());
#endif

TEST_CASE("zip", "[flow.zip]")
{
    REQUIRE(test_zip3());
    REQUIRE(test_zip2());
}

constexpr bool test_enumerate()
{
    using namespace std::string_view_literals;

    std::array arr{"a"sv, "b"sv, "c"sv};

    auto f = flow::enumerate(arr);
    int counter = 0;

    FLOW_FOR(auto m, f) {
        auto [item, idx] = m;

        if (not (item == arr[counter] && idx== counter)) {
            return false;
        }

        ++counter;
    }

    return true;
}
#if !COMPILER_IS_MSVC
static_assert(test_enumerate());
#endif

TEST_CASE("enumerate", "[flow.enumerate]")
{
    REQUIRE(test_enumerate());
}

}



#include <flow.hpp>

#include "catch.hpp"

namespace {

constexpr bool test_count_if()
{
    std::array arr{1, 2, 3, 4, 5};
    auto is_even = [](int i) { return i % 2 == 0; };

    return flow::from(arr).count_if(is_even) == 2;
}
static_assert(test_count_if());

}

TEST_CASE("Basic count_if", "[flow.count_if]")
{
    REQUIRE(test_count_if());
}
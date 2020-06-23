
#include <flow.hpp>

#include "catch.hpp"

#include <chrono>

namespace {

using namespace std::literals;

constexpr bool test_sum()
{
    bool success = flow::of(1, 2, 3, 4, 5).sum() == 15;

    success = success && (flow::sum(std::array{1u, 2u, 3u, 4u, 5u}) == 15u);

    success = success && (flow::of(1s, 2s, 3s, 4s, 5s).sum() == 15s);

    return success;
}
static_assert(test_sum());

TEST_CASE("flow::sum", "[flow.sum]")
{
    REQUIRE(flow::of(1, 2, 3, 4, 5).sum() == 15);

    REQUIRE(flow::sum(std::array{1u, 2u, 3u, 4u, 5u}) == 15u);

    REQUIRE(flow::of("a"s, "b"s, "c"s).sum() == "abc"s);

    REQUIRE(flow::sum(std::array{"a"s, "b"s, "c"s}) == "abc"s);
}

}

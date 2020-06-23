
#include <flow.hpp>

#include "catch.hpp"

namespace {

constexpr bool test_product()
{
    bool success = true;

    success = success && (flow::of(-1, 2, 3, 4, 5).product() == -120);

    success = success && (flow::product(std::array{-1, 2, 3, 4, 5}) == -120);

    success = success && (flow::of(2.0, 3.5, -1.0).product() == 2.0 * 3.5 * -1.0);

    success = success && (flow::product(std::array{2.0, 3.5, -1.0}) == 2.0 * 3.5 * -1.0);

    return success;
}
static_assert(test_product());

TEST_CASE("product()", "[flow.product]")
{
    REQUIRE(flow::of(-1, 2, 3, 4, 5).product() == -120);

    REQUIRE(flow::product(std::array{-1, 2, 3, 4, 5}) == -120);

    REQUIRE(flow::of(2.0, 3.5, -1.0).product() == 2.0 * 3.5 * -1.0);

    REQUIRE(flow::product(std::array{2.0, 3.5, -1.0}) == 2.0 * 3.5 * -1.0);
}

}

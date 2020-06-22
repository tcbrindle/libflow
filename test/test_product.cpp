
#include <flow.hpp>

#include "catch.hpp"

namespace {

static_assert(flow::of(-1, 2, 3, 4, 5).product() == -120);

static_assert(flow::product(std::array{-1, 2, 3, 4, 5}) == -120);

static_assert(flow::of(2.0, 3.5, -1.0).product() == 2.0 * 3.5 * -1.0);

static_assert(flow::product(std::array{2.0, 3.5, -1.0}) == 2.0 * 3.5 * -1.0);

TEST_CASE("product()", "[flow.product]")
{
    REQUIRE(flow::of(-1, 2, 3, 4, 5).product() == -120);

    REQUIRE(flow::product(std::array{-1, 2, 3, 4, 5}) == -120);

    REQUIRE(flow::of(2.0, 3.5, -1.0).product() == 2.0 * 3.5 * -1.0);

    REQUIRE(flow::product(std::array{2.0, 3.5, -1.0}) == 2.0 * 3.5 * -1.0);
}

}

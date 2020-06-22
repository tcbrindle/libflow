
#include <flow.hpp>

#include "catch.hpp"

namespace {

constexpr bool test_zip_with2()
{
    auto sum = [](auto... args) { return (args + ...); };

    return flow::ints(0)
        .zip_with(sum, flow::ints(10))
        .take(5)
        .equal(std::array{10, 12, 14, 16, 18});
}
static_assert(test_zip_with2());

constexpr bool test_zip_with3()
{
    auto prod = [](auto... args) { return (args * ...); };

    return flow::zip_with(prod, flow::ints(1), flow::ints(1), flow::ints(1))
                .take(5)
                .equal(std::array{1, 8, 27, 64, 125});
}
static_assert(test_zip_with3());

TEST_CASE("zip_with", "[flow.zip_with]")
{
    std::vector numbers = {0, 1, 2, 3};
    std::string letters = "abcd";

    auto out = flow::zip_with([](int i, char c) {
        return flow::of(c).cycle().take(i);
    }, numbers, letters).flatten().to_string();

    REQUIRE(out == "bccddd");
}

}
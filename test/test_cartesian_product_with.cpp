
#include "catch.hpp"

#include <flow.hpp>

#include <iostream>

namespace {

constexpr bool test_cartesian_product_with3()
{
    auto sum = [](auto&&... args) { return (FLOW_FWD(args) + ... ); };

    std::array longs{100L, 200L};
    auto ints = flow::ints().take(2);
    short const shorts[] = { 5, 10 };

    auto prod = flow::cartesian_product_with(sum, longs, std::move(ints), shorts);

    static_assert(std::is_same_v<flow::item_t<decltype(prod)>, long>);

    const auto required = std::array{
        105, 110,
        106, 111,
        205, 210,
        206, 211
    };

    return flow::equal(std::move(prod), required);
}
static_assert(test_cartesian_product_with3());

constexpr bool test_cartesian_product_with2()
{
    std::array floats{1.1f, 2.2f, 3.3f};
    auto ints = flow::ints().take(3);

    auto prod = flow::cartesian_product_with(std::plus<>{}, floats, std::move(ints));

    static_assert(std::is_same_v<flow::item_t<decltype(prod)>, float>);

    const auto required = std::array{
        1.1f, 2.1f, 3.1f,
        2.2f, 3.2f, 4.2f,
        3.3f, 4.3f, 5.3f
    };

    return flow::equal(std::move(prod), required);
}
static_assert(test_cartesian_product_with2());

TEST_CASE("Cartesian product with")
{
    REQUIRE(test_cartesian_product_with3());
    REQUIRE(test_cartesian_product_with2());
}

}
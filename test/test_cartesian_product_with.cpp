
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flow.hpp>

#include <iostream>

namespace {

constexpr bool test_cartesian_product_with3()
{
    auto sum = [](auto&&... args) { return (FLOW_FWD(args) + ... ); };

    std::array longs{100L, 200L};
    auto ints = flow::iota(0).take(2);
    std::array<short,2> const shorts = { 5, 10 };

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

constexpr bool test_cartesian_product_with_size3()
{
    const auto sink = [](auto...){};
    const std::array arr{1, 2, 3};

    {
        auto prod = flow::from(arr).cartesian_product_with(sink, arr, arr);

        static_assert(not flow::is_infinite_flow<decltype(prod)>);
        static_assert(flow::is_sized_flow<decltype(prod)>);

        if (prod.size() != 3 * 3 * 3) {
            return false;
        }
    }

    {
        auto prod = flow::from(arr).filter([](auto) { return true; })
                        .cartesian_product_with(sink, arr, arr);

        static_assert(not flow::is_infinite_flow<decltype(prod)>);
        static_assert(not flow::is_sized_flow<decltype(prod)>);
    }

    {
        auto prod = flow::from(arr).cartesian_product_with(sink, flow::ints(), arr);

        static_assert(flow::is_infinite_flow<decltype(prod)>);
        static_assert(not flow::is_sized_flow<decltype(prod)>);
    }

    return true;
}
static_assert(test_cartesian_product_with_size3());

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

constexpr bool test_cartesian_product_with_size2()
{
    const auto sink = [](auto...){};
    const std::array arr{1, 2, 3};

    {
        auto ints = flow::ints().take(10);

        auto prod = flow::from(arr).cartesian_product_with(sink, std::move(ints));

        static_assert(not flow::is_infinite_flow<decltype(prod)>);
        static_assert(flow::is_sized_flow<decltype(prod)>);

        if (prod.size() != 3 * 10) {
            return false;
        }
    }

    {
        auto ints = flow::ints().take_while([](auto) { return true; });

        auto prod = flow::from(arr).cartesian_product_with(sink, std::move(ints));

        static_assert(not flow::is_infinite_flow<decltype(prod)>);
        static_assert(not flow::is_sized_flow<decltype(prod)>);
    }

    {
        auto prod = flow::ints().cartesian_product_with(sink, arr);

        static_assert(flow::is_infinite_flow<decltype(prod)>);
        static_assert(not flow::is_sized_flow<decltype(prod)>);
    }

    return true;
}
static_assert(test_cartesian_product_with_size2());

TEST_CASE("Cartesian product with")
{
    REQUIRE(test_cartesian_product_with3());
    REQUIRE(test_cartesian_product_with_size3());

    REQUIRE(test_cartesian_product_with2());
    REQUIRE(test_cartesian_product_with_size2());
}

}

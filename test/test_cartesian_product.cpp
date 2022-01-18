
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flow.hpp>
#include <iostream>

constexpr bool test_cartesian_product3()
{
    using namespace std::string_view_literals;

    auto const arr = std::array{10, 20};
    auto ints = flow::ints().take(2);
    auto strs = flow::of{ "one"sv, "two"sv};

    auto prod = flow::cartesian_product(arr, std::move(ints), std::move(strs));

    using output_t = std::tuple<int const&, flow::dist_t, std::string_view&>;

    static_assert(std::is_same_v<flow::item_t<decltype(prod)>, output_t>);

    const auto output = std::array<std::tuple<int, flow::dist_t, std::string_view>, 8>{
        std::tuple{10, 0, "one"}, {10, 0, "two"},
        {10, 1, "one"}, {10, 1, "two"},
        {20, 0, "one"}, {20, 0, "two"},
        {20, 1, "one"}, {20, 1, "two"}
    };

    return prod.size() == 2 * 2 * 2 &&
            flow::equal(std::move(prod), output);
}
static_assert(test_cartesian_product3());

constexpr bool test_cartesian_product2()
{
    return flow::of{1, 2, 3}
                    .cartesian_product(flow::c_str("abc"))
                    .map([](auto p) {
                        auto [i, c] = p;
                        // Work around AppleClang bug
                        auto f = flow::of{c};
                        return std::move(f).cycle().take(i);
                    })
                    .flatten()
                    .equal(flow::c_str("abcaabbccaaabbbccc"));
}
static_assert(test_cartesian_product2());

TEST_CASE("cartesian_product()")
{
    REQUIRE(test_cartesian_product3());

    REQUIRE(test_cartesian_product2());
}

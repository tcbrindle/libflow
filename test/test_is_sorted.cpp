
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

namespace {

using namespace std::literals;

constexpr bool test_is_sorted()
{
    if (not flow::empty<int>().is_sorted()) {
        return false;
    }

    if (not flow::of(1, 2, 3, 4).is_sorted()) {
        return false;
    }

    if (not flow::is_sorted(std::array{1, 2, 3, 4})) {
        return false;
    }

    if (not flow::of(4.0, 3.0, 2.0, -1.0).is_sorted(std::greater<double>{})) {
        return false;
    }

    // Make sure we stop in the right place
    std::array arr{1, 2, 3, 4, 3, 2, 1};
    auto f = flow::from(arr);

    if (f.is_sorted()) {
        return false;
    }

    if (std::addressof(f.next().value()) != arr.data() + 5) {
        return false;
    }

    return true;
}
static_assert(test_is_sorted());

TEST_CASE("is_sorted", "[flow.is_sorted]")
{
    REQUIRE(flow::empty<int>().is_sorted());

    REQUIRE(flow::is_sorted(std::vector<int>{}));

    REQUIRE(flow::of(1, 2, 3, 4, 5).is_sorted());

    REQUIRE(flow::is_sorted("abcdef"s));

    REQUIRE_FALSE(flow::of(1, 2, 3, 4, 5).is_sorted(std::greater<>{}));

    REQUIRE_FALSE(flow::is_sorted(std::vector{1, 2, 3, 4, 5}, std::greater<>{}));

    REQUIRE_FALSE(flow::is_sorted("abcdef"s, std::greater<>{}));

    std::array arr{1, 2, 3, 4, 3, 2, 1};
    auto f = flow::from(arr);

    REQUIRE_FALSE(f.is_sorted());

    REQUIRE(std::addressof(f.next().value()) == arr.data() + 5);
}


}

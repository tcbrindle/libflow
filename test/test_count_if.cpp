
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_member_count_if()
{
    std::array arr{1, 2, 3, 4, 5};
    auto is_even = [](int i) { return i % 2 == 0; };

    if (flow::from(arr).count_if(is_even) != 2) {
        return false;
    }

    auto is_one = [](int i) { return i == 1; };
    return flow::of{1, 1, 1, 2, 3, 4, 1}.count_if(is_one) == 4;
}
static_assert(test_member_count_if());

constexpr bool test_nonmember_count_if()
{
    std::array arr{1, 2, 3, 4, 5};
    auto is_even = [](int i) { return i % 2 == 0; };

    if (flow::count_if(flow::from(arr), is_even) != 2) {
        return false;
    }

    auto is_one = [](int i) { return i == 1; };
    return flow::count_if(flow::of{1, 1, 1, 2, 3, 4, 1}, is_one) == 4;
}
static_assert(test_nonmember_count_if());

TEST_CASE("Member count_if()", "[flow.count_if]")
{
    REQUIRE(test_member_count_if());
}

TEST_CASE("Non-member count_if()", "[flow.count_if]")
{
    REQUIRE(test_nonmember_count_if());
}

}

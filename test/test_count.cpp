
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

using namespace std::string_view_literals;

constexpr bool test_member_nullary_count()
{
    auto arr = std::array{1, 2, 3, 4, 5};
    if (flow::from(arr).count() != (flow::dist_t) arr.size()) {
        return false;
    }

    // Empty flows return a count of zero
    return flow::empty<std::string_view>{}.count() == 0;
}
static_assert(test_member_nullary_count());

TEST_CASE("member nullary count()", "[flow.count]")
{
    REQUIRE(test_member_nullary_count());
    REQUIRE(flow::from("abcdef"sv).count() == 6);
}

constexpr bool test_nonmember_nullary_count()
{
    auto arr = std::array{1, 2, 3, 4, 5};
    if (flow::count(flow::from(arr)) != (flow::dist_t) arr.size()) {
        return false;
    }

    return flow::count(flow::empty<std::string_view>{}) == 0;
}
static_assert(test_nonmember_nullary_count());

TEST_CASE("nonmember nullary count()", "[flow.count]")
{
    REQUIRE(test_nonmember_nullary_count());
    REQUIRE(flow::count(flow::from("abcdef"sv)) == 6);
}

constexpr bool test_member_unary_count()
{
    auto arr = std::array{"a"sv, "b"sv, "c"sv};
    if (flow::from(arr).count("a") != 1) {
        return false;
    }
    if (flow::from(arr).count("zebedee") != 0) {
        return false;
    }
    return flow::empty<int>{}.count(1) == 0;
}
static_assert(test_member_unary_count());

TEST_CASE("member unary count()", "[flow.count]")
{
    REQUIRE(test_member_nullary_count());
    REQUIRE(flow::from("abcdef"sv).count('a') == 1);
}

constexpr bool test_nonmember_unary_count()
{
    auto arr = std::array{"a"sv, "b"sv, "c"sv};
    if (flow::count(flow::from(arr), "a") != 1) {
        return false;
    }
    if (flow::count(flow::from(arr), "zebedee") != 0) {
        return false;
    }
    return flow::count(flow::empty<int>{}, 1) == 0;
}
static_assert(test_nonmember_unary_count());

TEST_CASE("nonmember unary count()", "[flow.count]")
{
    REQUIRE(test_nonmember_nullary_count());
    REQUIRE(flow::count(flow::from("abcdef"sv), 'a') == 1);
}

}

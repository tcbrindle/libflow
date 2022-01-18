
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

namespace {

using namespace std::literals;

constexpr auto is_upper = [](char c) {
    return c >= 'A' && c <= 'Z';
};

constexpr bool test_all()
{
    // test empty flow
    if (not flow::empty<int>().all(flow::pred::positive)) {
        return false;
    }

    if (not flow::of(1, 2, 3, 4, 5).all(flow::pred::positive)) {
        return false;
    }

    if (not flow::all(std::array{1, 2, 3, 4, 5}, flow::pred::positive)) {
        return false;
    }

    if (not flow::c_str("ABCDEF").all(is_upper)) {
        return false;
    }

    if (not flow::all("ABCDEF"sv, is_upper)) {
        return false;
    }

    if (flow::c_str("ABcDEF").all(is_upper)) {
        return false;
    }

    if (flow::all("ABcDEF"sv, is_upper)) {
        return false;
    }

    return true;
}
static_assert(test_all());

// ...and now let's do all those again at run-time
TEST_CASE("all()", "[flow.all]")
{
    REQUIRE(flow::empty<int>().all(flow::pred::positive));

    REQUIRE(flow::all(std::vector<int>{}, flow::pred::positive));

    REQUIRE(flow::of(1, 2, 3, 4, 5).all(flow::pred::positive));

    REQUIRE(flow::all(std::vector{1, 2, 3, 4, 5}, flow::pred::positive));

    REQUIRE(flow::c_str("ABCDEF").all(is_upper));

    REQUIRE(flow::all("ABCDEF"s, is_upper));

    REQUIRE_FALSE(flow::all("ABcDEF"s, is_upper));
}


constexpr bool test_none()
{
    // test empty flow
    if (not flow::empty<int>().none(flow::pred::positive)) {
        return false;
    }

    if (not flow::of(1, 2, 3, 4, 5).none(flow::pred::negative)) {
        return false;
    }

    if (not flow::none(std::array{1, 2, 3, 4, 5}, flow::pred::negative)) {
        return false;
    }

    if (not flow::c_str("abcdef").none(is_upper)) {
        return false;
    }

    if (not flow::none("acbdef"sv, is_upper)) {
        return false;
    }

    if (flow::c_str("ABcDEF").none(is_upper)) {
        return false;
    }

    if (flow::none("ABcDEF"sv, is_upper)) {
        return false;
    }

    return true;
}
static_assert(test_none());

TEST_CASE("none()", "[flow.none]")
{
    REQUIRE(flow::empty<int>().none(flow::pred::positive));

    REQUIRE(flow::none(std::vector<int>{}, flow::pred::positive));

    REQUIRE(flow::of(1, 2, 3, 4, 5, 0).none(flow::pred::negative));

    REQUIRE(flow::none(std::vector{1, 2, 3, 4, 5, 0}, flow::pred::negative));

    REQUIRE(flow::c_str("abcdef").none(is_upper));

    REQUIRE(flow::none("abcdef"s, is_upper));

    REQUIRE_FALSE(flow::none("abCdef"s, is_upper));
}

constexpr bool test_any()
{
    // test empty flow
    if (flow::empty<int>().any(flow::pred::positive)) {
        return false;
    }

    if (not flow::of(1, 2, 3, 4, 5).any(flow::pred::positive)) {
        return false;
    }

    if (not flow::any(std::array{1, 2, 3, 4, 5}, flow::pred::positive)) {
        return false;
    }

    if (not flow::c_str("ABCDEF").any(is_upper)) {
        return false;
    }

    if (not flow::any("ABCDEF"sv, is_upper)) {
        return false;
    }

    if (flow::c_str("abcdef").any(is_upper)) {
        return false;
    }

    if (flow::any("abcdef"sv, is_upper)) {
        return false;
    }

    return true;
}
static_assert(test_any());

TEST_CASE("any()", "[flow.any]")
{
    REQUIRE_FALSE(flow::empty<int>().any(flow::pred::positive));

    REQUIRE_FALSE(flow::any(std::vector<int>{}, flow::pred::positive));

    REQUIRE(flow::of(1, 2, 3, 4, 5).any(flow::pred::positive));

    REQUIRE(flow::any(std::vector{1, 2, 3, 4, 5}, flow::pred::positive));

    REQUIRE(flow::c_str("ABCDEF").any(is_upper));

    REQUIRE(flow::any("ABCDEF"s, is_upper));

    REQUIRE_FALSE(flow::any("abcdef"s, is_upper));
}


}


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr auto ascii_case_insensitive = [](char a, char b) {
  auto tolower = [](char c) {
    if (c >= 97 && c <= 122) {
        return (char) (c - 32);
    }
    return c;
  };

  return tolower(a) == tolower(b);
};

constexpr bool test_member_contains()
{
    constexpr std::array arr{1, 2, 3, 4, 5};

    if (flow::from(arr).contains(999)) {
        return false;
    }

    {
        auto f = flow::from(arr);
        if (!f.contains(3)) {
            return false;
        }

        if (f.next().value() != 4) {
            return false;
        }
    }

    // Test with custom comparator
    {
        auto f = flow::c_str("hElLO wOrLd");
        if (!f.contains('e', ascii_case_insensitive)) {
            return false;
        }
        if (f.next().value() != 'l') {
            return false;
        }
    }

    return true;
}
static_assert(test_member_contains());

TEST_CASE("contains() member", "[flow.find]")
{
    REQUIRE(test_member_contains());
}

constexpr bool test_nonmember_contains()
{
    constexpr std::array arr{1, 2, 3, 4, 5};

    if (flow::contains(flow::from(arr), 999)) {
        return false;
    }

    {
        auto f = flow::from(arr);
        if (!flow::contains(f, 3)) {
            return false;
        }

        if (f.next().value() != 4) {
            return false;
        }
    }

    // Test with custom comparator
    {
        auto f = flow::c_str("hElLO wOrLd");
        if (!flow::contains(f, 'e', ascii_case_insensitive)) {
            return false;
        }
        if (f.next().value() != 'l') {
            return false;
        }
    }

    return true;
}
static_assert(test_nonmember_contains());

TEST_CASE("contains() non-member", "[flow.find]")
{
    REQUIRE(test_nonmember_contains());
}

}

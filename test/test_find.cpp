
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

constexpr bool test_member_find()
{
    constexpr std::array arr{1, 2, 3, 4, 5};

    {
        auto m = flow::from(arr).find(999);
        if (m) {
            return false;
        }
    }

    {
        auto f = flow::from(arr);
        auto m = f.find(3);
        if (std::addressof(m.value()) != arr.data() + 2) {
            return false;
        }

        if (f.next().value() != 4) {
            return false;
        }
    }

    // Test with custom comparator
    {
        auto f = flow::c_str("HeLlO wOrLd");
        auto m = f.find('l', ascii_case_insensitive);
        if (m.value() != 'L') {
            return false;
        }
        if (f.next().value() != 'l') {
            return false;
        }
    }

    return true;
}
static_assert(test_member_find());

TEST_CASE("find() member", "[flow.find]")
{
    REQUIRE(test_member_find());
}

constexpr bool test_nonmember_find()
{
    std::array arr{1, 2, 3, 4, 5};

    {
        auto m = flow::find(flow::from(arr), 999);
        if (m) {
            return false;
        }
    }

    {
        auto f = flow::from(arr);
        auto m = flow::find(f, 3);
        if (std::addressof(m.value()) != arr.data() + 2) {
            return false;
        }

        if (f.next().value() != 4) {
            return false;
        }
    }

    // Test with custom comparator
    {
        auto f = flow::c_str("HeLlO wOrLd");
        auto m = flow::find(f, 'l', ascii_case_insensitive);
        if (m.value() != 'L') {
            return false;
        }
        if (f.next().value() != 'l') {
            return false;
        }
    }

    return true;
}
static_assert(test_nonmember_find());

TEST_CASE("find() non-member", "[flow.find]")
{
    REQUIRE(test_nonmember_find());
}

}

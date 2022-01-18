
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <iostream>

namespace {

constexpr bool test_interleave()
{
    {
        auto b = flow::c_str("ABC")
                     .copy()
                     .interleave(flow::iota('1'))
                     .equal(flow::c_str("A1B2C3"));
        if (!b) {
            return false;
        }
    }

    {
        auto is_even = [](auto i) { return i % 2 == 0; };

        auto evens = flow::ints().filter(is_even);
        auto odds = flow::ints().filter(flow::pred::not_(is_even));

        bool b = flow::interleave(evens, odds)
            .take(10)
            .equal(flow::ints().take(10));
        if (!b) {
            return false;
        }
    }

    {
        bool b = flow::c_str("ABCDEF")
                     .interleave(flow::c_str(" ").cycle())
                     .equal(flow::c_str("A B C D E F "));
        if (!b) {
            return false;
        }
    }

    // test subflow
    {
        auto f = flow::of{1, 2, 3}
                     .interleave(flow::of{9}.cycle());
        (void) f.next();

        bool b =
            f.subflow().equal(flow::of(9, 2, 9, 3, 9)) &&
            f.equal(flow::of{9, 2, 9, 3, 9});

        if (!b) {
            return false;
        }
    }

    return true;
}
static_assert(test_interleave());

TEST_CASE("interleave", "[flow.interleave]")
{
    REQUIRE(test_interleave());
}

}


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_take()
{
    // Basic take
    {
        auto f = flow::ints(0, 10).take(5);

        static_assert(flow::is_flow<decltype(f)>);

        if (f.size() != 5) {
            return false;
        }

        if (not f.equal(flow::ints(0, 5))) {
            return false;
        }
    }

    // Take with an infinite flow
    {
        auto f = flow::ints().take(5);

        static_assert(!flow::is_infinite_flow<decltype(f)>);

        if (f.size() != 5) {
            return false;
        }

        if (not f.equal(flow::ints(0, 5))) {
            return false;
        }
    }

    // Take with an undersize flow
    {
        auto arr = {1, 2, 3};

        auto f = flow::take(arr, 1001);

        if (f.size() != 3) {
            return false;
        }

        if (not f.equal(arr)) {
            return false;
        }
    }

    // Take with subrange
    {
        auto f = flow::ints().take(5);

        if (f.subflow().size() != 5) {
            return false;
        }

        (void) f.next();

        if (not f.subflow().equal(flow::ints(1, 5))) {
            return false;
        }
    }

    return true;
}
static_assert(test_take());

TEST_CASE("Take", "[flow.take]")
{
    REQUIRE(test_take());
}

}

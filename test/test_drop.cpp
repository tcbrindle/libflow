
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_drop()
{
    // We can drop stuff
    {
        auto f = flow::ints().drop(5);

        static_assert(flow::is_flow<decltype(f)>);
        static_assert(flow::is_infinite_flow<decltype(f)>);

        if (f.next().value() != 5) {
            return false;
        }
    }

    // We can drop stuff, with a free-function
    {
        std::array arr{1, 2, 3, 4, 5};

        auto f = flow::drop(arr, 3);

        if (f.size() != 2) {
            return false;
        }

        if (not f.equal(flow::of{4, 5})) {
            return false;
        }
    }

    // We can drop everything
    {
        auto f = flow::ints(0, 10).drop(500);

        static_assert(flow::is_sized_flow<decltype(f)>);

        if (f.size() != 0) {
            return false;
        }

        if (f.next().has_value()) {
            return false;
        }
    }

    // We can drop nothing
    {
        auto f = flow::ints(0, 10).drop(0);

        static_assert(flow::is_sized_flow<decltype(f)>);

        if (f.size() != 10) {
            return false;
        }

        if (not f.equal(flow::ints(0, 10))) {
            return false;
        }
    }

    // We can advance dropped flows
    {
        auto f = flow::ints(0, 10).drop(5);
        // f is [5, 6, 7, 8, 9]

        if (f.advance(3).value() != 7) {
            return false;
        }
    }

    // We can drop multiple times
    {
        auto f = flow::ints(0, 10).drop(3).drop(2);

        if (f.size() != 5) {
            return false;
        }


        if (not f.equal(flow::ints(5, 10))) {
            return false;
        }
    }

    // We can take subflows of dropped flows
    {
        auto f = flow::ints(0, 10).drop(5);

        if (f.subflow().size() != 5) {
            return false;
        }

        if (not f.subflow().equal(flow::ints(5, 10))) {
            return false;
        }

        (void) f.next();

        if (not f.subflow().equal(flow::ints(6, 10))) {
            return false;
        }
    }

    return true;
}
static_assert(test_drop());

TEST_CASE("drop", "[flow.drop]")
{
    REQUIRE(test_drop());
}

}


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_cycle()
{
    // Basic cycle
    {
        auto f = flow::ints(0, 3).cycle();

        static_assert(flow::is_flow<decltype(f)>);
        static_assert(flow::is_infinite_flow<decltype(f)>);

        auto f2 = std::move(f).take(10);

        if (f2.size() != 10) {
            return false;
        }

        if (not f2.equal(flow::of(0, 1, 2, 0, 1, 2, 0, 1, 2, 0))) {
            return false;
        }
    }

    // Cycle with advance()
    {
        std::array arr{1, 2, 3};

        auto f = flow::cycle(arr);

        auto m = f.advance(1'000);

        if (m.value() != 1'000 % 3) {
            return false;
        }
    }

    // Cycle subflows
    {
        auto f = flow::of(1, 2, 3, 4, 5).cycle();

        (void) f.advance(5);

        if (f.subflow().next().value() != 1) {
            return false;
        }

        if (f.next().value() != 1) {
            return false;
        }
    }

    return true;
}
static_assert(test_cycle());

TEST_CASE("cycle", "[flow.cycle]")
{
    REQUIRE(test_cycle());
}

}


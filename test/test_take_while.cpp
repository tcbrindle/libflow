
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_take_while()
{
    // Basic take_while
    {
        auto f = flow::ints().take_while(flow::pred::lt(5));

        static_assert(flow::is_flow<decltype(f)>);

        if (not f.equal(flow::ints(0, 5))) {
            return false;
        }
    }

    // take_while doesn't filter after it should
    {
        auto f = flow::ints(0, 10).take_while(flow::pred::even);

        if (not f.equal(flow::of(0))) {
            return false;
        }
    }

    // We can take everything
    {
        auto yes = [](auto const&) { return true; };

        auto f = flow::ints(0, 10).take_while(yes);

        if (not f.equal(flow::ints(0, 10))) {
            return false;
        }
    }

    // We can take nothing
    {
        auto no = [](auto const&) { return false; };

        auto f = flow::ints(0, 10).take_while(no);

        if (f.next().has_value()) {
            return false;
        }
    }

    // Subflows work as expected
    {
        auto f = flow::ints(0, 10).take_while(flow::pred::lt(5));

        if (not f.subflow().equal(flow::ints(0, 5))) {
            return false;
        }

        (void) f.advance(3);

        if (f.subflow().equal(flow::ints(2, 5))) {
            return false;
        }
    }

    return true;
}
static_assert(test_take_while());

TEST_CASE("take while", "[flow.take_while]")
{
    REQUIRE(test_take_while());
}


}


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_drop_while()
{
    // Basic drop while
    {
        auto f = flow::ints(0, 10).drop_while(flow::pred::lt(5));

        static_assert(flow::is_flow<decltype(f)>);

        if (not f.equal(flow::ints(5, 10))) {
            return false;
        }
    }

    // drop_while doesn't filter after it should
    {
        auto f = flow::ints(0, 10).drop_while(flow::pred::even);

        if (not f.equal(flow::ints(1, 10))) {
            return false;
        }
    }

    // We can drop everything
    {
        auto yes = [](const auto&) { return true; };

        auto f = flow::ints(0, 10).drop_while(yes);

        if (f.next().has_value()) {
            return false;
        }
    }

    // We can drop nothing
    {
        auto no = [](const auto&) { return false; };

        auto f = flow::ints(0, 10).drop_while(no);

        if (not f.equal(flow::ints(0, 10))) {
            return false;
        }
    }

    // Subflows work as expected
    {
        auto f = flow::ints(0, 10).drop_while(flow::pred::even);

        if (not f.subflow().equal(flow::ints(1, 10))) {
            return false;
        }

        (void) f.next();

        if (not f.subflow().equal(flow::ints(2, 10))) {
            return false;
        }
    }

    return true;
}
static_assert(test_drop_while());

TEST_CASE("drop_while", "[flow.drop_while]")
{
    REQUIRE(test_drop_while());
}

}

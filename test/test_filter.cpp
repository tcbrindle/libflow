
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

struct Pair {
    int a;
    bool ok;

    constexpr int get() const { return a; }
    constexpr bool is_okay() const { return ok; }
};

constexpr bool test_filter()
{
    // Basic filtering
    {
        auto f = flow::ints(0, 10).filter(flow::pred::even);

        static_assert(flow::is_flow<decltype(f)>);
        static_assert(not flow::is_infinite_flow<decltype(f)>);
        static_assert(not flow::is_sized_flow<decltype(f)>);

        if (not f.equal(flow::ints(0, 10, 2))) {
            return false;
        }
    }

    // A predicate that is always true returns what it was given
    {
        auto yes = [](auto const&) { return true; };
        auto f = flow::ints(0, 5).filter(yes);

        if (not f.equal(flow::ints(0, 5))) {
            return false;
        }
    }

    // A predicate that is always false returns nothing
    {
        auto no = [](auto const&) { return false; };
        auto f = flow::ints(0, 10).filter(no);

        if (f.next().has_value()) {
            return false;
        }
    }

    // We can create subflows of filtered flows
    {
        auto f = flow::ints(0, 10).filter(flow::pred::even);

        if (f.subflow().count() != 5) {
            return false;
        }
    }

    // free-function flow::filter works okay
    {
        std::array arr{1, 2, 3, 4, 5};

        auto f = flow::filter(arr, flow::pred::even);

        if (not f.equal(flow::ints(2, 6, 2))) {
            return false;
        }
    }

    // We can pass a PMD to filter
    // (except GCC really doesn't like PMDs in constexpr)
#if !COMPILER_IS_GCC
    {
        std::array<Pair, 4> pairs = {
            Pair{1, true},
            {2, false},
            {3, true},
            {4, false}
        };

        auto f = flow::filter(pairs, &Pair::ok).map(&Pair::get);

        if (not f.equal(flow::of{1, 3})) {
            return false;
        }
    }
#endif

    // We can pass a PMF to a filter
    {
        std::array<Pair, 4> pairs = {
            Pair{1, true},
            {2, false},
            {3, true},
            {4, false}
        };

        auto f = flow::from(pairs).filter(&Pair::is_okay).map(&Pair::get);

        if (not f.equal(flow::of{1, 3})) {
            return false;
        }
    }

    return true;
}
static_assert(test_filter());

TEST_CASE("filter", "[flow.filter]")
{
    REQUIRE(test_filter());
}

}

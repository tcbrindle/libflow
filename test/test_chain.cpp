
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

struct input_only : flow::flow_base<input_only> {

    constexpr input_only() {}

    input_only(input_only&&) = default;
    input_only& operator=(input_only&&) = default;

    constexpr flow::maybe<int> next() { return {1}; }
};

constexpr bool test_chain3()
{
    auto arr1 = std::array{1, 2, 3};
    auto arr2 = std::array{4, 5, 6};
    auto arr3 = std::array{7, 8, 9};

    // Basic chain
    {
        auto f = flow::chain(arr1, arr2, arr3);

        if (f.size() != 9) {
            return false;
        }

        if (not f.equal(flow::ints(1, 10))) {
            return false;
        }
    }

    // Chain with infinite flows works as expected
    {
        auto f = flow::chain(flow::ints(0, 3), flow::ints(10, 13), flow::ints(100));

        static_assert(flow::is_infinite_flow<decltype(f)>);

        static_assert(!flow::is_sized_flow<decltype(f)>);

        auto f2 = std::move(f).take( 10);

        if (not f2.equal(flow::of(0, 1, 2, 10, 11, 12, 100, 101, 102, 103))) {
            return false;
        }
    }

    // Chain with subflows works okay
    {
        auto f = flow::chain(arr1, std::array{4, 5, 6}, arr3);

        if (not f.subflow().equal(flow::ints(1, 10))) {
            return false;
        }

        (void) f.advance(4); // put us in the second array

        if (not f.subflow().equal(flow::ints(5, 10))) {
            return false;
        }

        if (not f.equal(flow::ints(5, 10))) {
            return false;
        }
    }

    // Input-only flows do not cause compilation failures!
    {
        [[maybe_unused]] auto f = flow::chain(input_only{}, input_only{});

        static_assert(flow::is_flow<decltype(f)>);
        static_assert(not flow::is_multipass_flow<decltype(f)>);
    }

    // Empty flows are ignored
    {
        auto f = flow::chain(arr1, flow::empty<int&>(), flow::empty<int&>{}, arr2,
                             flow::empty<int&>(), arr3, flow::empty<int&>{});

        if (f.size() != 9) {
            return false;
        }

        if (f.count() != 9) {
            return false;
        }
    }

    // Overridden try_fold() behaves as expected
    {
        if(flow::chain(arr1, arr2, arr3).sum() != 45) {
            return false;
        }

        auto f = flow::chain(arr1, arr2, arr3);

        // Make sure we break correctly
        if (f.all(flow::pred::lt(7))) {
            return false;
        }

        if (f.next().value() != 8) {
            return false;
        }
    }

    return true;
}
static_assert(test_chain3());

// Same again, but this time testing the two-flow chain specialisation
constexpr bool test_chain2()
{
    auto arr1 = std::array{1, 2, 3, 4, 5};
    auto arr2 = std::array{6, 7, 8, 9};

    // Basic chain
    {
        auto f = flow::chain(arr1, arr2);

        if (f.size() != 9) {
            return false;
        }

        if (not f.equal(flow::ints(1, 10))) {
            return false;
        }
    }

    // Chain with infinite flows works as expected
    {
        auto f = flow::chain(flow::ints(0, 3), flow::ints(100));

        static_assert(flow::is_infinite_flow<decltype(f)>);

        static_assert(!flow::is_sized_flow<decltype(f)>);

        auto f2 = std::move(f).take( 5);

        if (not f2.equal(flow::of(0, 1, 2, 100, 101))) {
            return false;
        }
    }

    // Chain with subflows works okay
    {
        auto f = flow::chain(arr1, std::array{6, 7, 8, 9});

        if (not f.subflow().equal(flow::ints(1, 10))) {
            return false;
        }

        (void) f.advance(6); // put us in the second array

        if (not f.subflow().equal(flow::ints(7, 10))) {
            return false;
        }

        if (not f.equal(flow::ints(7, 10))) {
            return false;
        }
    }

    // Input-only flows do not cause compilation failures!
    {
        [[maybe_unused]] auto f = flow::chain(input_only{}, input_only{});

        static_assert(flow::is_flow<decltype(f)>);
        static_assert(not flow::is_multipass_flow<decltype(f)>);
    }

    // Overridden try_fold() behaves as expected
    {
        if(flow::chain(arr1, arr2).sum() != 45) {
            return false;
        }

        auto f = flow::chain(arr1, arr2);


        // Make sure we break correctly
        if (f.all(flow::pred::lt(7))) {
            return false;
        }

        if (f.next().value() != 8) {
            return false;
        }
    }

    return true;
}
static_assert(test_chain2());

TEST_CASE("Chain", "[flow.chain]")
{
    REQUIRE(test_chain3());
    REQUIRE(test_chain2());
}


}

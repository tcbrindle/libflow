
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <chrono>
#include <iostream>

namespace {

using namespace std::literals;

constexpr bool test_iota_basic()
{
    auto f = flow::iota(0u);

    static_assert(flow::is_infinite_flow<decltype(f)>);
    static_assert(!flow::is_sized_flow<decltype(f)>);

    return std::move(f)
                .take(5)
                .equal(flow::of(0u, 1u, 2u, 3u, 4u));
}
static_assert(test_iota_basic());

constexpr bool test_iota_bounded()
{
    {
        auto f = flow::iota('a', 'z' + 1);

        if (!f.equal(flow::c_str("abcdefghijklmnopqrstuvwxyz"))) {
            return false;
        }
    }

    // Iterators
    {
        std::array arr{1, 2, 3, 4, 5};

        auto f = flow::iota(arr.begin(), arr.end()).unchecked_deref();

        if (f.size() != 5) {
            return false;
        }

        if (!f.equal(arr)) {
            return false;
        }
    }

    // Chrono durations
    {
        auto f = flow::iota(0min, 1h);

        if (f.subflow().count() != 60) {
            return false;
        }

        if (flow::iota(1s, 1min).sum() != 29min + 30s) {
             return false;
        }
    }

    // Reversing
    {
        auto f = flow::iota(0, 5).reverse();

        if (f.size() != 5) {
            return false;
        }

        if (!f.equal(std::array{4, 3, 2, 1, 0})) {
            return false;
        }
    }

    return true;
}
static_assert(test_iota_bounded());

constexpr bool test_iota_step()
{
    // Test ints, counting up
    {
        auto f = flow::iota(1, 10, 2);
        if (f.size() != 5) {
            return false;
        }

        if (!f.equal(flow::of(1, 3, 5, 7, 9))) {
            return false;
        }
    }

    // Test ints, counting down
    {
        auto f = flow::iota(10, 0, -3);
        if (f.size() != 4) {
            return false;
        }

        if (!f.equal(flow::of(10, 7, 4, 1))) {
            return false;
        }
    }

    // Test chrono timepoints, positive increment
    {
        auto start = std::chrono::system_clock::time_point{};
        auto end = start + 10min;

        auto f = flow::iota(start, end, 2min);

        // Cannot call f.size() :-(
        if (f.subflow().count() != 5) {
            return false;
        }

        auto minmax = f.minmax();

        if (minmax->min != start) {
            return false;
        }

        if (minmax->max != start + 8min){
            return false;
        }
    }

    // Test chrono timepoints, negative increment
    {
        auto start = std::chrono::system_clock::time_point{} + 10min;
        auto end = std::chrono::system_clock::time_point{};

        auto f = flow::iota(start, end, -2min);

        // Cannot call f.size() :-(
        if (f.subflow().count() != 5) {
            return false;
        }

        bool b =
            f.next().value() == start &&
            f.next().value() == start - 2min &&
            f.next().value() == start - 4min &&
            f.next().value() == start - 6min &&
            f.next().value() == start - 8min &&
            not f.next().has_value();

        if (!b) {
            return false;
        }
    }

    return true;
}
static_assert(test_iota_step());

TEST_CASE("iota", "[flow.iota]")
{
    REQUIRE(test_iota_basic());
    REQUIRE(test_iota_bounded());
    REQUIRE(test_iota_step());
}

constexpr bool test_ints()
{
    // No args
    {
        auto f = flow::ints();
        using F = decltype(f);

        static_assert(std::is_same_v<flow::item_t<F>, flow::dist_t>);
        static_assert(flow::is_infinite_flow<F>);
        static_assert(flow::is_multipass_flow<F>);
        static_assert(not flow::is_sized_flow<F>);

        if (f.next().value() != 0) {
            return false;
        }
    }

    // start
    {
        auto f = flow::ints(3);

        if (f.next().value() != 3) {
            return false;
        }
    }

    // start and end
    {
        auto max = std::numeric_limits<flow::dist_t>::max();

        auto f = flow::ints(max - 10, max);

        if (f.size() != 10) {
            return false;
        }

        if (f.count() != 10) {
            return false;
        }
    }

    // start, end, positive step
    {
        auto f = flow::ints(0, 100, 11);

        if (f.size() != 10) {
            return false;
        }

        if (not f.equal(flow::of(0, 11, 22, 33, 44, 55, 66, 77, 88, 99))) {
            return false;
        }
    }

    // start, end, negative step
    {
        auto f = flow::ints(99, -1, -11);

        if (f.size() != 10) {
            return false;
        }

        if (not f.equal(flow::of(99, 88, 77, 66, 55, 44, 33, 22, 11, 0))) {
            return false;
        }
    }

    return true;
}
static_assert(test_ints());

TEST_CASE("ints", "[flow.ints]")
{
    REQUIRE(test_ints());
}

}

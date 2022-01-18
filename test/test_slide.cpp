
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <set>

namespace {

template <typename Ints>
constexpr bool test_slide_no_partial(Ints ints)
{
    // Test window_size == 1, step_size == 1
    // Expects: [ [1], [2], [3], [4], [5] ]
    // (this "unflattens" the flow)
    {
        auto f = flow::slide(ints, 1);

        for (int i : ints) {
            auto inner = f.next().value();
            if (not inner.equal(flow::of(i))) {
                return false;
            }
        }

        // Make sure we've exhausted the flow
        if (f.next().has_value()) {
            return false;
        }

        // Test round-tripping with flatten()
        if (not flow::from(ints).slide(1).flatten().equal(ints)) {
            return false;
        }
    }

    // Test window_size == 2, step_size == 1
    // Expects [ [1, 2], [2, 3], [3, 4], [4, 5] ]
    {
        auto f = flow::slide(ints, 2);

        bool success =
            f.next().value().equal(flow::of(1, 2)) &&
            f.next().value().equal(flow::of(2, 3)) &&
            f.next().value().equal(flow::of(3, 4)) &&
            f.next().value().equal(flow::of(4, 5)) &&
            not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Test window_size == 2, step_size == 2
    // Expects [ [1, 2], [3, 4] ]
    // (This is like chunk, but with no final partial chunk)
    {
        auto f = flow::slide(ints, 2, 2);

        bool success =
            f.next().value().equal(flow::of(1, 2)) &&
            f.next().value().equal(flow::of(3, 4)) &&
            not f.next().has_value();
        if (!success) {
            return false;
        }
    }

    // Test window_size == 3, step_size == 1
    // Expects [ [1, 2, 3], [2, 3, 4], [3, 4, 5] ]
    {
        auto f = flow::from(ints).slide(3);

        bool success =
            f.next().value().equal(flow::of(1, 2, 3)) &&
            f.next().value().equal(flow::of(2, 3, 4)) &&
            f.next().value().equal(flow::of(3, 4, 5)) &&
            not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Test window_size == 3, step_size == 2
    // Expects [ [1, 2, 3], [3, 4, 5] ]
    {
        auto f = flow::from(ints).slide(3, 2);

        bool success =
            f.next().value().equal(flow::of(1, 2, 3)) &&
            f.next().value().equal(flow::of(3, 4, 5)) &&
            not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Test window_size == 1, step_size == 2
    // Expects [ [1], [3], [5] ]
    // This is equivalent to an "unflattened" stride
    {
        auto f = flow::from(ints).slide(1, 2);

        bool success =
            f.next().value().equal(flow::of(1)) &&
            f.next().value().equal(flow::of(3)) &&
            f.next().value().equal(flow::of(5)) &&
            not (f.next().has_value());

        if (!success) {
            return false;
        }

        // Make sure a "flattened" version of the above is really the same as
        // doing stride
        if (not flow::from(ints).slide(1, 2).flatten().equal(flow::from(ints).stride(2))) {
            return false;
        }
    }

    // Test window_size == 5
    // Expects [ [1, 2, 3, 4, 5] ] (regardless of step size)
    {
        auto f1 = flow::from(ints).slide(5, 55);
        bool success = f1.next().value().equal(ints) &&
                       not f1.next().has_value();

        if (!success) {
            return false;
        }

        // same again, this time with flatten
        auto f2 = flow::from(ints).slide(5, 55).flatten();
        if (not f2.equal(ints)) {
            return false;
        }
    }

    // Test window_size > 5
    // Expects [] (i.e. no "partial" window)
    {
        auto f = flow::from(ints).slide(6);
        if (f.next().has_value()) {
            return false;
        }
    }

    // Ensure we can take a subflow of a slide_adaptor
    {
        auto f = flow::from(ints).slide(3, 2);
        (void) f.next(); // drop the first value
        auto s = f.subflow();

        bool success =
            f.next().value().equal(flow::of{3, 4, 5}) &&
            not f.next().has_value() &&
            s.next().value().equal(flow::of{3, 4, 5}) &&
            not s.next().has_value();

        if (!success) {
            return false;
        }
    }

    return true;
}

template <typename Ints>
constexpr bool test_slide_partial(const Ints ints)
{
    // Test window_size == 1, step_size == 1
    // Expects: [ [1], [2], [3], [4], [5] ]
    // (this "unflattens" the flow)
    {
        auto f = flow::from(ints).slide(1, 1, true);

        for (int i : ints) {
            auto inner = f.next().value();
            if (not inner.equal(flow::of(i))) {
                return false;
            }
        }

        // Make sure we've exhausted the flow
        if (f.next().has_value()) {
            return false;
        }

        // Test round-tripping with flatten()
        if (not flow::from(ints).slide(1, 1, true).flatten().equal(ints)) {
            return false;
        }
    }

    // Test window_size == 2, step_size == 1
    // Expects [ [1, 2], [2, 3], [3, 4], [4, 5], [5] ]
    {
        auto f = flow::from(ints).slide(2, 1, true);

        bool success =
            f.next().value().equal(flow::of(1, 2)) &&
            f.next().value().equal(flow::of(2, 3)) &&
            f.next().value().equal(flow::of(3, 4)) &&
            f.next().value().equal(flow::of(4, 5)) &&
            f.next().value().equal(flow::of(5)) &&
            not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Test window_size == 3, step_size == 1
    // Expects [ [1, 2, 3], [2, 3, 4], [3, 4, 5], [4, 5], [5] ]
    {
        auto f = flow::from(ints).slide(3, 1, true);

        bool success =
            f.next().value().equal(flow::of(1, 2, 3)) &&
            f.next().value().equal(flow::of(2, 3, 4)) &&
            f.next().value().equal(flow::of(3, 4, 5)) &&
            f.next().value().equal(flow::of(4, 5)) &&
            f.next().value().equal(flow::of(5)) &&
            not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Test window_size == 3, step_size == 2
    // Expects [ [1, 2, 3], [3, 4, 5], [5] ]
    {
        auto f = flow::from(ints).slide(3, 2, true);

        bool success =
            f.next().value().equal(flow::of(1, 2, 3)) &&
            f.next().value().equal(flow::of(3, 4, 5)) &&
            f.next().value().equal(flow::of(5)) &&
            not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Test window_size == 1, step_size == 2
    // Expects [ [1], [3], [5] ]
    // This is equivalent to an "unflattened" step_by
    {
        auto f = flow::from(ints).slide(1, 2, true);

        bool success =
            f.next().value().equal(flow::of(1)) &&
                f.next().value().equal(flow::of(3)) &&
                f.next().value().equal(flow::of(5)) &&
                not (f.next().has_value());

        if (!success) {
            return false;
        }

        // Make sure a "flattened" version of the above is really the same as
        // doing step_by
        if (not flow::from(ints).slide(1, 2, true).flatten().equal(flow::from(ints).stride(2))) {
            return false;
        }
    }

    // Test window_size == 5, step_size == 1
    // Expects [ [1, 2, 3, 4, 5], [2, 3, 4, 5], [3, 4, 5], [4, 5], [5] ]
    {
        auto f1 = flow::from(ints).slide(5, 1, true);
        bool success =
            f1.next().value().equal(ints) &&
            f1.next().value().equal(flow::from(ints).drop(1)) &&
            f1.next().value().equal(flow::from(ints).drop(2)) &&
            f1.next().value().equal(flow::from(ints).drop(3)) &&
            f1.next().value().equal(flow::from(ints).drop(4)) &&
            not f1.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Test window_size > 5, step_size == 1
    // Expects [ [1, 2, 3, 4, 5], [2, 3, 4, 5], [3, 4, 5], [4, 5], [5] ]
    // (same as window_size == 5)
    {
        auto f1 = flow::from(ints).slide(6, 1, true);
        bool success =
            f1.next().value().equal(ints) &&
                f1.next().value().equal(flow::from(ints).drop(1)) &&
                f1.next().value().equal(flow::from(ints).drop(2)) &&
                f1.next().value().equal(flow::from(ints).drop(3)) &&
                f1.next().value().equal(flow::from(ints).drop(4)) &&
                not f1.next().has_value();

        if (!success) {
            return false;
        }
    }

    // Ensure we can take a subflow of a slide_adaptor
    {
        auto f = flow::from(ints).slide(3, 2, true);
        (void) f.next(); // drop the first value
        auto s = f.subflow();

        bool success =
            f.next().value().equal(flow::of{3, 4, 5}) &&
                f.next().value().equal(flow::of(5)) &&
                not f.next().has_value() &&
                s.next().value().equal(flow::of{3, 4, 5}) &&
                s.next().value().equal(flow::of(5)) &&
                not s.next().has_value();

        if (!success) {
            return false;
        }
    }

    return true;
}

// constexpr tests with array
static_assert(test_slide_no_partial(std::array{1, 2, 3, 4, 5}));
static_assert(test_slide_partial(std::array{1, 2, 3, 4, 5}));

TEST_CASE("slide", "[flow.slide]")
{
    // Test with vector
    REQUIRE(test_slide_no_partial(std::vector{1, 2, 3, 4, 5}));
    REQUIRE(test_slide_partial(std::vector{1, 2, 3, 4, 5}));

    // Test with set, just for fun
    REQUIRE(test_slide_no_partial(std::set{1, 2, 3, 4, 5}));
    REQUIRE(test_slide_partial(std::set{1, 2, 3, 4, 5}));

    // Just do something silly
    auto str = flow::c_str("abcd")
                   .slide(4, 1, true)
                   .flatten()
                   .to_string();
    REQUIRE(str == "abcdbcdcdd");
}


}

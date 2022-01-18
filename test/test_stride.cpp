
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_stride()
{
    // Basic stride
    {
        auto f = flow::ints(0, 10).stride(2);

        static_assert(flow::is_flow<decltype(f)>);

        if (f.size() != 5) {
            return false;
        }

        if (not f.equal(flow::ints(0, 10, 2))) {
            return false;
        }
    }

    // Non-divisible size
    {
        auto f = flow::ints(0, 10).stride(3);

        if (f.size() != flow::ints(0, 10, 3).size()) {
            return false;
        }

        if (not f.equal(flow::of(0, 3, 6, 9))) {
            return false;
        }
    }

    // Stride with infinite flow
    {
        auto f = flow::ints().stride(2);

        static_assert(flow::is_infinite_flow<decltype(f)>);
        static_assert(!flow::is_sized_flow<decltype(f)>);

        if (not std::move(f).take(5).equal(flow::ints(0, 10, 2))) {
            return false;
        }
    }

    // Stride of 1
    {
        auto f = flow::ints(0, 10).stride(1);

        if (not f.equal(flow::ints(0, 10))) {
            return false;
        }
    }

    // Oversize stride
    {
        auto f = flow::ints(0, 10).stride(1001);

        if (f.next().value() != 0) {
            return false;
        }

        if (f.next().has_value()) {
            return false;
        }
    }

    // Stride subflows
    {
        auto f = flow::ints(0, 10).stride(3);

        if (f.subflow().size() != 4) {
            return false;
        }

        (void) f.next();

        if (not f.subflow().equal(flow::of(3, 6, 9))) {
            return false;
        }
    }

    return true;
}
static_assert(test_stride());

TEST_CASE("stride", "[flow.stride]")
{
    REQUIRE(test_stride());
}

}

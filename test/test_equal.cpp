
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_equal()
{
    std::array arr1{1, 2, 3, 4, 5};

    // Basic test
    if (not flow::equal(arr1, arr1)) {
        return false;
    }

    if (not flow::equal(arr1, flow::iota(1LL).take(5))) {
        return false;
    }

    // Test with custom comparator
    if (not flow::equal(arr1, flow::ints(2).take(5), flow::less{})) {
        return false;
    }

    // Test with flows of different lengths
    if (flow::equal(arr1, flow::from(arr1).take(4))) {
        return false;
    }
    if (flow::from(arr1).take(4).equal(arr1)) {
        return false;
    }

    return true;
}
static_assert(test_equal());

TEST_CASE("equal()", "[flow.equal]")
{
    REQUIRE(test_equal());
}

}


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <sstream>

namespace {

constexpr bool test_inspect()
{
    std::array in{1, 2, 3, 4, 5};
    int last = 99;

    auto f = flow::from(in)
        .inspect([&last] (auto const& v) {
            last = v;
        });


    for (int i = 0; i < 5; i++) {
        auto m = f.next();
        if (last != in[i]) {
            return false;
        }

        // Make sure we're yielding the exact same elements we had originally
        if (std::addressof(*m) != in.data() + i) {
            return false;
        }
    }

    return true;
}
static_assert(test_inspect());

TEST_CASE("inspect()", "[flow.inspect]")
{
    REQUIRE(test_inspect());

    std::ostringstream ss;
    auto func = [&ss] (const auto& val) { ss << val; };

    // Very very complex pipeline...
    auto sum = flow::ints(0, 10)
        .filter([](int i) { return i % 2 == 0; })
        .inspect(func)
        .map([](int i) { return i * i; })
        .sum();
    REQUIRE(sum == 2*2 + 4*4 + 6*6 + 8*8);
    REQUIRE(ss.str() == "02468");
}

}

// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "flow.hpp"

#include "catch.hpp"

#include <iostream>

namespace {
#ifndef _MSC_VER
constexpr bool test_rvalue_ints()
{
    int counter = 0;
    FLOW_FOR(int i, flow::of{ 0, 1, 2, 3 }) {
        if (i != counter++) {
            return false;
        }
    }
    return true;
}
static_assert(test_rvalue_ints());

constexpr bool test_lvalue_ints()
{
    int counter = 0;
    int _0 = 0;
    int _1 = 1;
    int _2 = 2;

    FLOW_FOR(int i, flow::of(_0, _1, _2)) {
        if (i != counter++) {
            return false;
        }
    }
    return true;
}
static_assert(test_lvalue_ints());
#endif
}

TEST_CASE("flow::of rvalue strings", "[flow.of]")
{
    using namespace std::string_literals;
    auto strings = std::array{"A"s, "B"s, "C"s};

    int counter = 0;

    FLOW_FOR(const auto& s, flow::of("A"s, "B"s, "C"s)) {
        REQUIRE(s == strings[counter++]);
    }
}

TEST_CASE("flow::of lvalue strings", "[flow.of]") {
    using namespace std::string_literals;
    auto strings = std::array{"A"s, "B"s, "C"s};

    int counter = 0;

    FLOW_FOR(const auto& s, flow::of(strings[0], strings[1], strings[2])) {
        REQUIRE(s == strings[counter++]);
    }
}

TEST_CASE("flow::of rvalue unique_ptr", "[flow.of]") {
    auto mkuniq = [i = 0] () mutable { return std::make_unique<int>(i++); };
    int counter = 0;

    FLOW_FOR(auto& p, flow::of{mkuniq(), mkuniq(), mkuniq()}) {
        REQUIRE(*p == counter++);
    }
}

TEST_CASE("flow::of lvalue unique_ptr", "[flow.of]") {
    auto mkuniq = [i = 0] () mutable { return std::make_unique<int>(i++); };
    std::array arr{mkuniq(), mkuniq(), mkuniq()};

    int counter = 0;

    FLOW_FOR(auto& r, flow::of(std::ref(arr[0]), std::ref(arr[1]), std::ref(arr[2]))) {
        REQUIRE(r.get() == arr[counter++]);
    }
}
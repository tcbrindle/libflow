
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <iostream>

namespace {

void takes_any(flow::any_flow<int&> f) {
    auto vec = std::move(f).to_vector();
    REQUIRE((vec == std::vector{1, 2, 3}));
}

void takes_any_ref(flow::any_flow_ref<char> f) {
    std::string str = std::move(f).to_string();
    REQUIRE(str == "abcdef");
}

}

TEST_CASE("any_flow", "[flow.any_flow]")
{
    takes_any(flow::of(1, 2, 3));
}

TEST_CASE("any_flow_ref", "[flow.any_flow_ref]")
{
    auto flow = flow::iota('a').take(6);
    takes_any_ref(flow);
}

// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <list>
#include <map>
#include <iostream>

TEST_CASE("to_vector()", "[flow.to]")
{
    auto vec = flow::iota(0, 5).to_vector();
    REQUIRE((vec == std::vector<int>{0, 1, 2, 3, 4}));
}

TEST_CASE("to_vector() (unique_ptr)", "[flow.to]")
{


}

TEST_CASE("to_vector<T>()", "[flow.to]")
{
    auto vec = flow::ints(0, 5).to_vector<float>();
    REQUIRE((vec == std::vector<float>{0, 1, 2, 3, 4}));
}

TEST_CASE("to<std::map>()", "[flow.to]")
{
    using namespace std::string_literals;

    auto map = flow::of("a"s, "b"s, "c"s)
                   .zip(flow::ints())
        .to<std::map<std::string, int>>();

    REQUIRE((map == std::map<std::string, int>{{"a", 0}, {"b", 1}, {"c", 2}}));
}

TEST_CASE("to_string()", "[flow.to]")
{
    auto str = flow::of('a', 'b', 'c').to_string();
    REQUIRE(str == "abc");
}

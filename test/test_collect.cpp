
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <map>
#include <set>
#include <unordered_map>


TEST_CASE("collect() to vector", "[flow.collect]")
{
    std::vector<int> vec = flow::of(1, 2, 3).collect();
    REQUIRE((vec == std::vector{1, 2, 3}));
}

TEST_CASE("collect() to string", "[flow.collect]")
{
    std::string str = flow::iota('a').take(26).collect();
    REQUIRE(str == "abcdefghijklmnopqrstuvwxyz");
}

TEST_CASE("collect() to set", "[flow.collect]")
{
    std::set<int> set = flow::ints(1, 4).collect();
    REQUIRE(set == std::set{1, 2, 3});
}

TEST_CASE("collect() to map", "[flow.collect]")
{
    std::map<std::string, int> map =
        flow::of<std::string, 3>("A", "B", "C").zip(flow::ints(1))
            .collect();

    REQUIRE((map == std::map<std::string, int>{{"A", 1}, {"B", 2}, {"C", 3}}));
}

TEST_CASE("collect() to unordered_map", "[flow.collect]")
{
    std::unordered_map<std::string, int> map =
        flow::of<std::string, 3>("A", "B", "C").zip(flow::ints(1))
            .collect();

    REQUIRE((map == std::unordered_map<std::string, int>{{"A", 1}, {"B", 2}, {"C", 3}}));
}
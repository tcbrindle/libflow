
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <list>
#include <map>
#include <set>

namespace {

constexpr bool test()
{
    {
        auto in = std::array{1, 2, 3, 4, 5};
        auto out = std::array{5, 4, 3, 2, 1};

        if (!flow::from(in).reverse().equal(out)) {
            return false;
        }
    }

    {
        auto in = std::array{1, 2, 3, 4, 5};
        auto out = std::array{5, 4, 3, 2, 1};

        if (!flow::reverse(in).equal(out)) {
            return false;
        }
    }

    {
        auto in = std::array{1, 2, 3, 4, 5};
        auto twice = [](int i) { return i + i; };
        auto out = std::array{10, 8, 6, 4, 2};

        if (!flow::reverse(in).map(twice).equal(out)) {
            return false;
        }
    }

    return true;
}
static_assert(test());

TEST_CASE("reverse() works for vectors")
{
    auto vec = std::vector<int>{1, 2, 3, 4, 5};

    REQUIRE((flow::from(vec).reverse().to_vector() ==
             std::vector<int>{5, 4, 3, 2, 1}));
}

TEST_CASE("reverse() works for lists")
{
    auto list = std::list<int>{1, 2, 3, 4, 5};

    REQUIRE(flow::reverse(list).equal(flow::of{5, 4, 3, 2, 1}));
}

TEST_CASE("reverse() works for sets")
{
    const auto set = std::set{3, 2, 5, 4, 1};

    REQUIRE(flow::reverse(set).equal(flow::of{5, 4, 3, 2, 1}));
}

TEST_CASE("reverse() works for maps")
{
    using namespace std::string_view_literals;

    const auto map = std::map<int, std::string>{
        {1, "one"},
        {2, "two"},
        {3, "three"}
    };

    REQUIRE(flow::reverse(map).keys().equal(flow::of{3, 2, 1}));

    REQUIRE(flow::values(map).reverse().equal(flow::of{"three"sv, "two"sv, "one"sv}));
}

}
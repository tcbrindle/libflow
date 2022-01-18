
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <sstream>

namespace {

// Sigh, std::pair::operator= isn't constexpr?!
struct int_pair {
    int first, second;

    friend constexpr bool operator==(const int_pair& lhs, const int_pair& rhs)
    {
        return lhs.first == rhs.first && lhs.second && rhs.second;
    }

    friend constexpr bool operator!=(const int_pair& lhs, const int_pair& rhs)
    {
        return !(lhs == rhs);
    }
};

constexpr auto compare_second = [](auto const& p, auto const& q) {
    return p.second < q.second;
};

constexpr std::array<int_pair, 6> pairs{
    int_pair{1, -1},
    {2, -2},
    {3, -2},
    {4, 99},
    {5, 99},
    {6, 0}
};

/*
 * Min tests
 */

static_assert(flow::from(pairs).min(compare_second).value() == int_pair{2, -2});

static_assert(flow::min(pairs, compare_second).value() == int_pair{2, -2});

constexpr bool test_min()
{
    if (flow::of(-1, 10, 999, -123).min().value() != -123){
        return false;
    }

    if (flow::min(std::array{-1, 10, 999, -123}).value() != -123) {
        return false;
    }

    if (flow::from(pairs).min(compare_second).value() != int_pair{2, -2}) {
        return false;
    }

    // Make sure we don't have UB capturing a reference to an item
    auto m = flow::of(1, -3, 2, 5).min();
    const int& r = *m;

    return r == -3;
}
static_assert(test_min());

TEST_CASE("min", "[flow.min]")
{
    REQUIRE_FALSE(flow::empty<int>().min().has_value());

    REQUIRE(flow::of(3).min().value() == 3);

    REQUIRE(flow::of(-1, 10, 999, -123).min().value() == -123);

    REQUIRE(flow::min(std::array{-1, 10, 999, -123}).value() == -123);

    REQUIRE(flow::from(pairs).min(compare_second).value().first == 2);

    REQUIRE(flow::min(pairs, compare_second).value() == int_pair{2, -2});

    std::istringstream iss{"-1 10 999 -123"};
    REQUIRE(flow::from_istream<int>(iss).min().value() == -123);
}

/*
 * Max tests
 */

static_assert(flow::from(pairs).max(compare_second).value() == int_pair{5, 99});

static_assert(flow::max(pairs, compare_second).value() == int_pair{5, 99});

constexpr bool test_max()
{
    if (flow::of(-1, 10, 999, -123).max().value() != 999){
        return false;
    }

    if (flow::max(std::array{-1, 10, 999, -123}).value() != 999) {
        return false;
    }

    if (flow::from(pairs).max(compare_second).value() != int_pair{5, 99}) {
        return false;
    }

    // Make sure we don't have UB capturing a reference to an item
    auto m = flow::of(1, -3, 2, 5).max();
    const int& r = *m;

    return r == 5;
}
static_assert(test_max());

TEST_CASE("max", "[flow.max]")
{
    REQUIRE_FALSE(flow::empty<int>().max().has_value());

    REQUIRE(flow::of(3).max().value() == 3);

    REQUIRE(flow::of(-1, 10, 999, -123).max().value() == 999);

    REQUIRE(flow::max(std::array{-1, 10, 999, -123}).value() == 999);

    REQUIRE(flow::from(pairs).max(compare_second).value().first == 5);

    REQUIRE(flow::max(pairs, compare_second).value() == int_pair{5, 99});

    std::istringstream iss{"-1 10 999 -123"};
    REQUIRE(flow::from_istream<int>(iss).max().value() == 999);
}

/*
 * Minmax tests
 */

static_assert(flow::from(pairs).minmax(compare_second).value() ==
              flow::minmax_result<int_pair>{int_pair{2, -2}, int_pair{5, 99}});

constexpr bool test_minmax()
{
    {
        auto res = flow::of(-1, 10, 999, -123).minmax().value();
        if (res != flow::minmax_result<int>{-123, 999}) {
            return false;
        }
    }

    {
        auto res = flow::minmax(std::array{-1, 10, 999, -123}).value();
        if (res != flow::minmax_result<int>{-123, 999}) {
            return false;
        }
    }

    return true;
}
static_assert(test_minmax());

TEST_CASE("minmax", "[flow.minmax]")
{
    REQUIRE_FALSE(flow::empty<int>().minmax().has_value());

    REQUIRE((flow::of(3).minmax().value() == flow::minmax_result<int>{3, 3}));

    {
        auto res = flow::of(-1, 10, 999, -123).minmax().value();
        REQUIRE(res.min == -123);
        REQUIRE(res.max == 999);
    }

    {
        auto res = flow::minmax(std::array{-1, 10, 999, -123}).value();
        REQUIRE(res.min == -123);
        REQUIRE(res.max == 999);
    }

    {
        auto res = flow::from(pairs).minmax(compare_second).value();
        REQUIRE(res.min == int_pair{2, -2});
        REQUIRE(res.max == int_pair{5, 99});
    }

    {
        auto res = flow::minmax(pairs, compare_second).value();
        REQUIRE(res.min == int_pair{2, -2});
        REQUIRE(res.max == int_pair{5, 99});
    }

    {
        std::istringstream iss{"-1 10 999 -123"};
        REQUIRE(flow::from_istream<int>(iss).minmax().value() ==
                    flow::minmax_result<int>{-123, 999});
    }
}

}
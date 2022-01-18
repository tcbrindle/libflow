
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <string_view>

namespace {

using namespace std::string_view_literals;

constexpr bool test_map_lambda()
{
    constexpr auto sq = [](int i) { return i * i; };
    const std::array arr{1, 2, 3};

    {
        auto f = flow::from(arr).map(sq);

        if (not std::move(f).equal(flow::of(1, 4, 9))) {
            return false;
        }
    }

    {
        auto f = flow::map(arr, sq);

        if (not std::move(f).equal(flow::of(1, 4, 9))) {
            return false;
        }
    }

    {
        if (not flow::of(1, 2, 3).map(sq).equal(flow::of(1, 4, 9))) {
            return false;
        }
    }

    return true;
}
static_assert(test_map_lambda());

constexpr int plus_one(int i) { return i + 1; }

constexpr bool test_map_function()
{
    constexpr std::array arr{1, 2, 3};

    if (not flow::from(arr).map(plus_one).equal(flow::of(2, 3, 4))) {
        return false;
    }

    if (not flow::map(arr, plus_one).equal(flow::of(2, 3, 4))) {
        return false;
    }

    return true;
}
static_assert(test_map_function());

constexpr bool test_map_pmf()
{
    constexpr std::array strings = {
        "a"sv, "bb"sv, "ccc"sv
    };

    constexpr std::array<std::size_t, 3> sizes = {1, 2, 3};

    // We're not supposed to take the address of a stdlib member function, but never mind
    if (not flow::from(strings).map(&std::string_view::size).equal(sizes)) {
        return false;
    }

    if (not flow::map(strings, &std::string_view::size).equal(sizes)) {
        return false;
    }

    return true;
}
static_assert(test_map_pmf());

struct int_pair {
    int first;
    int second;
};

constexpr bool test_map_pmd()
{
    constexpr std::array arr = {
        int_pair{1, 1},
        int_pair{2, 2},
        int_pair{3, 3}
    };

    if (not flow::from(arr).map(&int_pair::second).equal(flow::of(1, 2, 3))) {
        return false;
    }

    if (not flow::map(arr, &int_pair::second).equal(flow::of(1, 2, 3))) {
        return false;
    }

    return true;
}

#if !COMPILER_IS_GCC
static_assert(test_map_pmd());
#endif

constexpr bool test_map_identity()
{
    constexpr auto identity = [](auto&& val) -> decltype(auto) { return FLOW_FWD(val); };

    const std::array arr{1, 2, 3};

    {
        if (not flow::from(arr).map(identity).equal(flow::from(arr))) {
            return false;
        }
    }

    {
        if (not flow::map(arr, identity).map(identity).equal(arr)) {
            return false;
        }
    }

    {
        auto f = flow::from(arr).map(identity);

        for (int i = 0; i < 3; i++) {
            auto m = f.next();
            if (std::addressof(*m) != arr.data() + i) {
                return false;
            }
        }
    }

    {
        auto f = flow::map(arr, identity);

        for (int i = 0; i < 3; i++) {
            auto m = f.next();
            if (std::addressof(*m) != arr.data() + i) {
                return false;
            }
        }
    }

    {
        auto f1 = flow::from(arr);
        auto f = flow::map(f1, identity);

        for (int i = 0; i < 3; i++) {
            auto m = f.next();
            if (std::addressof(*m) != arr.data() + i) {
                return false;
            }
        }
    }

    return true;
}
static_assert(test_map_identity());

constexpr bool test_map_subflow()
{
    auto f = flow::map(std::array{1, 2, 3, 4, 5},
                       [](int i) { return i * i; });

    if (not f.subflow().equal(flow::of{1, 4, 9, 16, 25})) {
        return false;
    }

    if (not (f.equal(flow::of{1, 4, 9, 16, 25}))) {
        return false;
    }

    return true;
}
static_assert(test_map_subflow());

TEST_CASE("map()", "[flow.map]")
{
    REQUIRE(test_map_lambda());
    REQUIRE(test_map_function());
    REQUIRE(test_map_pmf());
    REQUIRE(test_map_pmd());
    REQUIRE(test_map_identity());
    REQUIRE(test_map_subflow());
}

}

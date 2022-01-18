
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <iostream>

namespace {

#if !COMPILER_IS_MSVC
constexpr bool test_flatten()
{
    auto f = [] {
        return flow::of(flow::of(1), flow::of(2), flow::of(3)).flatten();
    };

    auto is_one = [](int i) { return i == 1; };

    static_assert(f().contains(1));
    static_assert(f().count() == 3);
    static_assert(f().count(1) == 1);
    static_assert(f().min().value() == 1);
    static_assert(f().max().value() == 3);
    static_assert(f().is_sorted());
    static_assert(not f().all(is_one));
    static_assert(not f().none(is_one));
    static_assert(f().any(is_one));
    static_assert(f().filter(is_one).count() == 1);

    return true;
}
static_assert(test_flatten());
#endif

TEST_CASE("flatten() rvalue range", "[flow.flatten]")
{
    auto vec = flow::ints(1)
                   .map([](int i) { return flow::of{i}; })
                   .take(5)
                   .flatten()
                   .to_vector();

    REQUIRE((vec == std::vector{1, 2, 3, 4, 5}));
}

TEST_CASE("flatten() lvalue range", "[flow.flatten]")
{
    auto arr = std::array{flow::of(1), flow::of(2), flow::of(3)};
    auto vec = flow::from(arr).flatten().to_vector();

    REQUIRE((vec == std::vector{1, 2, 3}));
}

TEST_CASE("flatten(), then count", "[flow.flatten]")
{
    auto arr = std::array{flow::of(1), flow::of(2), flow::of(3)};

    auto cnt = flow::from(arr).flatten().count();
    REQUIRE(cnt == 3);
}

TEST_CASE("flatten(), then min", "[flow.flatten]")
{
    const auto vec_of_vecs = std::vector{
        std::vector{1, 2, 3}, std::vector{4, 5, 6}, std::vector{7, 8, 9}};

    auto f = [&vec_of_vecs] { return flow::from(vec_of_vecs).flatten(); };

    REQUIRE(f().count() == 9);
    REQUIRE(f().is_sorted());
    REQUIRE(f().min().value() == 1);
    REQUIRE(f().max().value() == 9);
}

TEST_CASE("flatten() subflows", "[flow.flatten]")
{
    auto flow_of_vecs = flow::of{std::vector{1, 2, 3}, std::vector{4, 5, 6, 7},
                                 std::vector{8, 9}};

    auto f = flow::flatten(std::move(flow_of_vecs));

    REQUIRE(f.subflow().next().value() == 1);

    // Advance into the second flow
    (void) f.advance(4);

    REQUIRE(f.subflow().next().value() == 5);
    REQUIRE(f.next().value() == 5);
}

constexpr bool test_flat_map()
{
    auto func = [](int i) { return flow::ints(1).take(i); };

    {
        auto f = flow::flat_map(flow::of{3, 2, 1}, func);

        (void) f.next();

        if (not f.subflow().equal(flow::of{2, 3, 1, 2, 1})) {
            return false;
        }

        if (not f.equal(flow::of{2, 3, 1, 2, 1})) {
            return false;
        }
    }

    return true;
}
static_assert(test_flat_map());

TEST_CASE("flat map", "[flow.flat_map]")
{
    REQUIRE(test_flat_map());
}

}

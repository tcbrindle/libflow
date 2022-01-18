
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <array>
#include <iostream>

namespace {

struct wrapper {
    int i;

    constexpr operator int() const { return i; }
};

struct move_only {
    int i{};

    constexpr move_only() = default;
    constexpr explicit move_only(int i) : i(i)  {}
    constexpr move_only(move_only&&) = default;
    constexpr move_only& operator=(move_only&&) = default;

    constexpr operator int() const { return i; }
};

template <typename T, int N>
struct test_flow : flow::flow_base<test_flow<T, N>> {

    constexpr test_flow()
    {
        for (int i = 0; i < N; i++) {
            array[i] = T{i};
        }
    }

    constexpr auto next() -> flow::maybe<T>
    {
        if (counter < N) {
           return {std::move(array[counter++])};
        }
        return {};
    }

private:
    std::array<T, N> array{};
    int counter = 0;
};

// A constexpr version of std::equal
template <typename Rng1, typename Rng2>
constexpr bool do_equal(const Rng1& rng1, const Rng2& rng2)
{
    auto it1 = std::begin(rng1);
    const auto s1 = std::end(rng1);
    auto it2 = std::begin(rng2);
    const auto s2 = std::end(rng2);

    while (it1 != s1 && it2 != s2) {
        if (*it1 != *it2) {
            return false;
        }
        ++it1; ++it2;
    }

    return (it1 == s1) == (it2 == s2);
}

template <typename Rng1, typename Rng2>
constexpr bool equal(const Rng1& rng1, const Rng2& rng2)
{
    return do_equal(rng1, rng2);
}

template <typename Rng1, typename T>
constexpr bool equal(const Rng1& rng1, std::initializer_list<T> ilist)
{
    return do_equal(rng1, ilist);
}

template <typename T>
constexpr bool test_while_loop_iteration()
{
    auto f = test_flow<T, 3>{};
    constexpr int out[] = {0, 1, 2};

    int counter = 0;
    while (auto m = f.next()) {
        if (*m != out[counter++]) {
            return false;
        }
    }

    return true;
}

static_assert(test_while_loop_iteration<int>());
static_assert(test_while_loop_iteration<move_only>());
#ifndef _MSC_VER
template <typename T>
constexpr bool test_for_loop_iteration()
{
    constexpr std::size_t sz = 3;
    auto f = test_flow<T, sz>{};
    int out[sz] = {};
    int counter = 0;

    for (flow::maybe<T> m = f.next(); m ; m = f.next()) {
        out[counter++] = *m;
    }

    return equal(out, {0, 1, 2});
}
static_assert(test_for_loop_iteration<int>());
static_assert(test_for_loop_iteration<move_only>());
#endif

template <typename T>
constexpr bool test_for_macro_iteration()
{
    constexpr std::size_t sz = 3;
    auto f = test_flow<T, sz>{};
    int out[sz] = {};
    int counter = 0;

    FLOW_FOR(auto&& m, f) {
        out[counter++] = m;
    }

    return equal(out, {0, 1, 2});
}
static_assert(test_for_macro_iteration<int>());
static_assert(test_for_macro_iteration<move_only>());

}


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow/core/predicates.hpp>

#include <array>
#include <string_view>

namespace {

namespace pred = flow::pred;
using namespace std::string_view_literals;

// constexpr, ranged std::all_of/none_of/any_of
auto all_of = [](const auto& rng, auto pred) {
    for (const auto& el : rng) {
        if (!pred(el)) {
            return false;
        }
    }
    return true;
};

auto none_of = [](const auto& rng, auto pred) { return all_of(rng, !pred); };

constexpr bool test_comparators()
{
    constexpr std::array ones{1, 1, 1, 1, 1, 1};
    constexpr std::array twos{2, 2, 2, 2, 2, 2};
    constexpr std::array negatives{-1.0, -2.0, -3.0, -4.0, -5.0};

    static_assert(all_of(ones, pred::eq(1)));
    static_assert(all_of(ones, pred::neq(22)));
    static_assert(all_of(ones, pred::lt(2)));
    static_assert(all_of(ones, pred::leq(1)));
    static_assert(all_of(ones, pred::gt(0)));
    static_assert(all_of(ones, pred::geq(1)));

    static_assert(all_of(ones, pred::positive));
    static_assert(none_of(ones, pred::negative));
    static_assert(all_of(ones, pred::nonzero));

    static_assert(all_of(negatives, pred::lt(0)));
    static_assert(none_of(negatives, pred::gt(0)));
    static_assert(all_of(negatives, pred::leq(-1)));
    static_assert(all_of(negatives, pred::geq(-5)));

    static_assert(none_of(negatives, pred::positive));
    static_assert(all_of(negatives, pred::negative));
    static_assert(all_of(negatives, pred::nonzero));

    static_assert(all_of(twos, pred::even));
    static_assert(none_of(ones, pred::even));
    static_assert(all_of(ones, pred::odd));
    static_assert(none_of(twos, pred::odd));

    return true;
}
static_assert(test_comparators());


constexpr bool test_combiners()
{
    {
        constexpr auto hello_or_world =
            pred::either(pred::eq("hello"sv), pred::eq("world"sv));

        static_assert(hello_or_world("hello"sv));
        static_assert(hello_or_world("world"sv));
        static_assert(!hello_or_world("goodbye"sv));
    }

    {
        constexpr auto hello_or_world =
            pred::eq("hello"sv) || pred::eq("world"sv);

        static_assert(hello_or_world("hello"));
        static_assert(hello_or_world("world"));
        static_assert(!hello_or_world("goodbye"));
    }

    {
        constexpr auto short_ = [](auto s) { return s.size() < 10; };
        constexpr auto shouty = [](auto s) {
            return all_of(s, [](char c) { return c >= 'A' && c <= 'Z'; });
        };

        constexpr auto short_and_shouty = pred::both(short_, shouty);

        static_assert(short_and_shouty("HELLO"sv));
        static_assert(!short_and_shouty("WHAT A LOVELY DAY WE'RE HAVING"sv));
        static_assert(!short_and_shouty("hello?"sv));
    }

    {
        constexpr auto hot = pred::eq("hot");
        constexpr auto cold = pred::eq("cold");
        constexpr auto tepid = pred::neither(hot, cold);

        static_assert(tepid("lukewarm"sv));
        static_assert(!tepid("hot"sv));
        static_assert(!tepid("cold"sv));
    }

    {
        constexpr auto in_names = pred::in("Adam"sv, "Barbara"sv, "Charles"sv);

        static_assert(in_names("Adam"));
        static_assert(in_names("Barbara"));
        static_assert(in_names("Charles"));
        static_assert(!in_names("Zacharia"));
    }

    return true;
}
static_assert(test_combiners());

}


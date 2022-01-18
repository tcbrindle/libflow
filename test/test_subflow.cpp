
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <deque>
#include <forward_list>
#include <memory>

namespace {

constexpr bool test_subflow()
{
    auto f = flow::from(std::array{1, 2, 3});
    f.next();

    auto s = f.subflow();
    if (s.next().value() != 2) {
        return false;
    }

    if (f.next().value() != 2) {
        return false;
    }

    return true;
}
static_assert(test_subflow());

constexpr auto identity = [](auto i) { return i; };

using F1 = decltype(flow::of(1, 2, 3).map(identity));

static_assert(flow::is_multipass_flow<F1>);

using F2 = decltype(flow::from_istream<int>(std::declval<std::istream&>()).map(identity));

static_assert(not flow::is_multipass_flow<F2>);

struct my_flow : flow::flow_base<my_flow> {
    auto next() -> flow::maybe<int>;
};

static_assert(flow::is_flow<my_flow>);

static_assert(flow::is_multipass_flow<my_flow>);

template <typename C>
void test_container(C& container)
{
    // First, with lvalues
    {
        auto f = flow::from(container);

        REQUIRE(**f.next() == 1);

        auto s = f.subflow();

        REQUIRE(**f.next() == 2);
        REQUIRE(**s.next() == 2);
        REQUIRE(**s.next() == 3);
        REQUIRE_FALSE(s.next().has_value());
        REQUIRE(**f.next() == 3);
        REQUIRE_FALSE(f.next().has_value());
    }

    // Then with rvalues
    {
        auto f = flow::from(std::move(container));

        REQUIRE(**f.next() == 1);

        auto s = f.subflow();

        REQUIRE(**f.next() == 2);
        REQUIRE(**s.next() == 2);
        REQUIRE(**s.next() == 3);
        REQUIRE_FALSE(s.next().has_value());
        REQUIRE(**f.next() == 3);
        REQUIRE_FALSE(f.next().has_value());
    }
}

TEST_CASE("subflow flow::from()", "[flow.subflow]")
{
    REQUIRE(test_subflow());

    // Random-access path
    {
        std::deque<std::unique_ptr<int>> d;
        d.push_front(std::make_unique<int>(1));
        d.push_back(std::make_unique<int>(2));
        d.emplace_back(new int{3});

        test_container(d);
    }

    // Forward path
    {
        std::forward_list<std::unique_ptr<int>> list{};

        list.push_front(std::make_unique<int>(3));
        list.push_front(std::make_unique<int>(2));
        list.emplace_front(new int{1});

        test_container(list);
    }

    // Check the input path won't work
    // Does the STL have a handy ready-made input range to test?
    {
        struct R {
            std::istream_iterator<int> begin() { return {}; }
            std::istream_iterator<int> end() { return {}; }
        };

        static_assert(flow::detail::is_stl_range<R>);

        using F = decltype(flow::from(std::declval<R>()));

        static_assert(flow::is_flow<F>);
        static_assert(not flow::is_multipass_flow<F>);
    }
}

}

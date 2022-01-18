
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include "macros.hpp"

#include <map>
#include <optional>

namespace {

using namespace std::literals;

/*
 * as() tests
 */
struct Base {};
struct Derived : Base {};

constexpr bool test_as()
{
    // Test casting double to int
    {
        auto f1 = flow::iota(1.0).take(5).as<int>();

        if (not f1.equal(flow::ints(1, 6))) {
            return false;
        }

        auto f2 = flow::as<int>(std::array{1.0, 2.0, 3.0, 4.0, 5.1});

        if (not f2.equal(flow::ints(1, 6))) {
            return false;
        }
    }

    // Test casting to const ref doesn't copy items
    {
        std::array arr{1, 2, 3, 4, 5};

        auto f1 = flow::from(arr).as<const int&>();

        for (int& i : arr) {
            auto m = f1.next();
            if (std::addressof(i) != std::addressof(*m)) {
                return false;
            }
        }
    }

    // Test casting Derived& to Base&
    {
        std::array<Derived, 3> arr{};

        auto f1 = flow::from(arr).as<Base const&>();

        for (const Base& b : arr) {
            auto m = f1.next();

            if (std::addressof(b) != std::addressof(*m)) {
                return false;
            }
        }
    }

    return true;
}
static_assert(test_as());

TEST_CASE("flow::as()", "[flow.as]")
{
    REQUIRE(test_as());

    // Test calling constructors with flow::as
    {
        auto vec = flow::of("a"sv, "b"sv, "c"sv).as<std::string>().to_vector();

        REQUIRE((vec == std::vector{"a"s, "b"s, "c"s}));
    }

    // Test calling conversion operators with flow::as
    {
        std::array strings = {"a"s, "b"s, "c"s};

        auto vec = flow::as<std::string_view>(strings).to_vector();

        REQUIRE(vec == std::vector{"a"sv, "b"sv, "c"sv});
    }
}

/*
 * unchecked_deref()
 */

constexpr bool test_unchecked_deref()
{
    int a = 1, b = 2, c = 3;

    // Test dereffing pointers
    {
        auto f = flow::of(&a, &b, &c).unchecked_deref();

        if (not f.equal(flow::of(1, 2, 3))) {
            return false;
        }
    }

    // Test dereffing maybes
    {
        std::array<flow::maybe<int&>, 3> arr{a, b, c};

        auto f = flow::unchecked_deref(arr);

        bool success =
            &f.next().value() == &a  &&
            &f.next().value() == &b  &&
            &f.next().value() == &c &&
            not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    return true;
}
static_assert(test_unchecked_deref());

constexpr bool test_unchecked_deref_optional()
{
    auto arr = std::array<std::optional<int>, 3>{1, 2, 3};

    auto f = flow::unchecked_deref(arr);

    return f.next().value() == 1 &&
            f.next().value() == 2 &&
            f.next().value() == 3 &&
            not f.next().has_value();
}
// FIXME: GCC doesn't like this for some reason (but Clang is fine)
#if !COMPILER_IS_GCC
static_assert(test_unchecked_deref_optional());
#endif

TEST_CASE("flow.unchecked_deref", "[flow.unchecked_deref]")
{
    REQUIRE(test_unchecked_deref());
    REQUIRE(test_unchecked_deref_optional());

    // Test dereffing unique_ptrs
    {
        auto ptrs = std::array{
            std::make_unique<int>(1),
            std::make_unique<int>(2),
            std::make_unique<int>(3)
        };

        REQUIRE(flow::unchecked_deref(ptrs).equal(flow::of(1, 2, 3)));
    }

    // Test dereffing shared pointers
    {
        auto ptrs = std::array{
            std::make_shared<int>(1),
            std::make_shared<int>(2),
            std::make_shared<int>(3)
        };

        REQUIRE(flow::unchecked_deref(ptrs).equal(flow::of(1, 2, 3)));
    }
}

/*
 * copy() tests
 */

constexpr bool test_copy()
{
    const auto arr = std::array{1, 2, 3};

    auto f = flow::copy(arr);
    static_assert(std::is_same_v<flow::item_t<decltype(f)>, int>);

    for (auto i : arr) {
        if (i != *f.next()) {
            return false;
        }
    }

    return !f.next().has_value();
}
static_assert(test_copy());

TEST_CASE("flow::copy()", "[flow.copy]")
{
    REQUIRE(test_copy());
}


/*
 * move() tests
 */

struct MoveOnly {
    constexpr MoveOnly(int i) : val(i) {}

    constexpr MoveOnly(MoveOnly&& other)
        : val(other.val)
    {
        other.val = 0;
    }

    constexpr MoveOnly& operator=(MoveOnly&& other)
    {
        val = other.val;
        other.val = 0;
        return *this;
    }

    ~MoveOnly() = default;

    int val;
};

constexpr auto test_move()
{
    std::array<MoveOnly, 3> arr{1, 2, 3};

    auto f = flow::move(arr);
//    static_assert(std::is_same_v<flow::item_t<decltype(f)>, MoveOnly&&>);

    int i = 0;
    FLOW_FOR(auto m, f) {
        if (arr[i++].val != 0) {
            return false;
        }

        if (m.val != i) {
            return false;
        }
    }

    return true;
}
static_assert(test_move());

TEST_CASE("flow::move()", "[flow.move]")
{
    REQUIRE(test_move());

    auto ptrs = std::array{
        std::make_unique<int>(1),
        std::make_unique<int>(2),
        std::make_unique<int>(3)
    };

    auto vec = flow::move(ptrs).to_vector();

    REQUIRE(*vec[0] == 1);
    REQUIRE(*vec[1] == 2);
    REQUIRE(*vec[2] == 3);

    for (auto& p : ptrs) {
        REQUIRE_FALSE((bool) p);
    }
}

/*
 * as_const() tests
 */
struct Immovable {
    Immovable() = default;
    Immovable(Immovable&&) = delete;
    Immovable& operator=(Immovable&&) = delete;
};

constexpr bool test_as_const()
{
    // Check that mutable lrefs become const lrefs
    {
        std::array arr{1, 2, 3};

        [[maybe_unused]] auto f = flow::as_const(arr);

        static_assert(std::is_same_v<flow::item_t<decltype(f)>, int const&>);
    }

    // Check that const lrefs stay const lrefs
    {
        std::array const arr{1, 2, 3};

        auto f1 = flow::from(arr);
        auto&& f2 = std::move(f1).as_const();

        if (&f1 != &f2) { // Check that we're not even really moving
            return false;
        }

        static_assert(std::is_same_v<flow::item_t<decltype(f2)>, int const&>);
    }

    // Check that non-references don't change
    {
        [[maybe_unused]] auto f = flow::ints().as_const();

        static_assert(std::is_same_v<flow::item_t<decltype(f)>, flow::dist_t>);
    }

    // Check that we're not accidentally copying or moving anything
    {
        auto arr = std::array<Immovable, 3>{};

        auto f = flow::from(arr).as_const();

        static_assert(std::is_same_v<flow::item_t<decltype(f)>, Immovable const&>);

        std::move(f).for_each([](Immovable const&) { /* pass */ });
    }

    return true;
}
static_assert(test_as_const());

TEST_CASE("as_const()", "[flow.as_const]")
{
    REQUIRE(test_as_const());
}

/*
 * elements() tests
 */
constexpr bool test_elements()
{
    using triple = std::tuple<int, int, int>;

    std::array arr = {
        triple{1, 2, 3},
        triple{4, 5, 6},
        triple{7, 8, 9}
    };

    return flow::elements<2>(arr).equal(flow::of{3, 6, 9});
}
static_assert(test_elements());

TEST_CASE("elements()", "[flow.elements]")
{
    REQUIRE(test_elements());
}

/*
 * keys()/values() tests
 */

TEST_CASE("keys()/values()", "[flow.keys] [flow.values]")
{
    auto map = std::map<int, std::string>{
        {1, "one"},
        {2, "two"},
        {3, "three"}
    };

    REQUIRE((flow::keys(map).to_vector() == std::vector{1, 2, 3}));

    REQUIRE((flow::values(map).to_vector() == std::vector<std::string>{"one", "two", "three"}));
}


}

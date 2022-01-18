
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include <string>

#include "catch.hpp"

namespace {

template <typename T>
struct test {

    struct convertible_to_T {
        operator T&() &;
        operator T const& () const&;
        operator T&&() &&;
        operator T const&& () const&&;
    };

    using M = flow::maybe<T>;

    static_assert(std::is_same_v<typename M::value_type, T>);

    // Default construction
    static_assert(std::is_nothrow_default_constructible_v<M>);

    // Construction from value
    static_assert(std::is_constructible_v<M, T>);
    static_assert(std::is_constructible_v<M, T const>);
    static_assert(std::is_constructible_v<M, T&>);
    static_assert(std::is_constructible_v<M, T const&>);
    static_assert(std::is_constructible_v<M, T&&>);
    static_assert(std::is_constructible_v<M, T const&&>);

    // If T is an lvalue reference type, we can't construct an M from a temporary
    static_assert(std::is_lvalue_reference_v<T> == !std::is_constructible_v<M, std::remove_reference_t<T>>);

    // Converting construction is "deleted"
    static_assert(!std::is_constructible_v<M, convertible_to_T>);
    static_assert(!std::is_constructible_v<M, convertible_to_T const>);
    static_assert(!std::is_constructible_v<M, convertible_to_T&>);
    static_assert(!std::is_constructible_v<M, convertible_to_T const&>);
    static_assert(!std::is_constructible_v<M, convertible_to_T&&>);
    static_assert(!std::is_constructible_v<M, convertible_to_T const&&>);

    // Copy construction and copy-assignment
    static_assert(std::is_copy_constructible_v<M> == std::is_copy_constructible_v<T>);
    static_assert(std::is_copy_assignable_v<M> ==
        (std::is_copy_assignable_v<T> || std::is_reference_v<T>));
    //static_assert(std::is_trivially_copyable_v<M> ==
    //    (std::is_trivially_copyable_v<T> || std::is_reference_v<T>));

    // Move construction and move-assignment
    static_assert(std::is_move_constructible_v<M>);
    static_assert(std::is_nothrow_move_constructible_v<M>);
    static_assert(std::is_trivially_move_constructible_v<M>
        == std::is_trivially_move_constructible_v<T>);
    static_assert(std::is_move_assignable_v<M>);
    static_assert(std::is_nothrow_move_assignable_v<M>
        == (std::is_nothrow_move_assignable_v<T> || std::is_reference_v<T>));
    static_assert(std::is_trivially_move_assignable_v<M> ==
        (std::is_trivially_move_assignable_v<T> || std::is_reference_v<T>));

    // Destruction
    static_assert(std::is_trivially_destructible_v<M>
        == std::is_trivially_destructible_v<T>);

    // Swap
    static_assert(std::is_nothrow_swappable_v<M>);

    // Dereference
    static_assert(std::is_same_v<decltype(*std::declval<M&>()), std::add_lvalue_reference_t<T>>);
    static_assert(std::is_same_v<decltype(*std::declval<M const&>()),
            std::add_lvalue_reference_t<std::add_const_t<T>>>);
    static_assert(std::is_same_v<decltype(*std::declval<M&&>()),
            std::add_rvalue_reference_t<T>>);
    static_assert(std::is_same_v<decltype(*std::declval<M const&&>()),
            std::add_rvalue_reference_t<std::add_const_t<T>>>);

};

struct not_copy_assignable {
    not_copy_assignable() = default;
    not_copy_assignable(const not_copy_assignable&) = default;
    not_copy_assignable& operator=(const not_copy_assignable&) = delete;
    not_copy_assignable& operator=(not_copy_assignable&&) = default;
};


constexpr bool compile_tests()
{
    // Trivial type
    (void) test<int>{};
    (void) test<int&>{};
    (void) test<int const&>{};

    // Well-behaved, non-trivial type
    (void) test<std::string>{};
    (void) test<std::string&>{};
    (void) test<std::string const&>{};

    // Move-only type
    (void) test<std::unique_ptr<int>>{};
    (void) test<std::unique_ptr<int>&>{};
    (void) test<std::unique_ptr<int> const&>{};

    // Copy constructable but not copy-assignable
    (void) test<not_copy_assignable>{};
    (void) test<not_copy_assignable&>{};
    (void) test<not_copy_assignable const&>{};

    return true;
}
static_assert(compile_tests());

TEST_CASE("maybe")
{
    auto m = flow::maybe<std::unique_ptr<int>>{};
    REQUIRE_FALSE(m);
    flow::maybe<std::unique_ptr<int>> m2{std::make_unique<int>(3)};
    REQUIRE(m2);
    REQUIRE(**m2 == 3);
    m = std::move(m2);
    REQUIRE(m);
    REQUIRE(**m == 3);
}


}


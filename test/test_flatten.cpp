
#include <flow.hpp>

#include "catch.hpp"

#include <iostream>

namespace {

constexpr bool test_flatten()
{
    auto f = [] {
         return flow::of(flow::of(1), flow::of(2), flow::of(3)).flatten();
    };

    auto is_one = [] (int i) { return i == 1; };

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

}

TEST_CASE("flatten() rvalue range", "[flow.flatten]")
{
    auto vec = flow::ints(1)
        .map([](int i) { return flow::of{i};})
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

    auto cnt = flow::from(arr)
        .flatten()
        .count();
    REQUIRE(cnt == 3);
}

TEST_CASE("flatten(), then min", "[flow.flatten]")
{
    auto vec_of_vecs = std::vector{
        std::vector{1, 2, 3},
        std::vector{4, 5, 6},
        std::vector{7, 8, 9}
    };

    auto f = [&vec_of_vecs] {
        return flow::from(vec_of_vecs)
            .map([](auto& vec) { return flow::from(vec); })
            .flatten();
    };

    REQUIRE(f().count() == 9);
    REQUIRE(f().is_sorted());
    REQUIRE(f().min().value() == 1);
    REQUIRE(f().max().value() == 9);
}

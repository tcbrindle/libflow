
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_zip_with2()
{
    auto sum = [](auto... args) { return (args + ...); };

    return flow::ints(0)
        .zip_with(sum, flow::ints(10))
        .take(5)
        .equal(std::array{10, 12, 14, 16, 18});
}
static_assert(test_zip_with2());

constexpr bool test_zip_with3()
{
    auto prod = [](auto... args) { return (args * ...); };

    return flow::zip_with(prod, flow::ints(1), flow::ints(1), flow::ints(1))
                .take(5)
                .equal(std::array{1, 8, 27, 64, 125});
}
static_assert(test_zip_with3());

constexpr bool test_zip_with2_subflow()
{
    auto sum = [](auto... args) { return (args + ...); };

    auto f = flow::ints(0, 5).zip_with(sum, flow::ints(10));

    (void) f.next();

    if (not f.subflow().equal(std::array{12, 14, 16, 18})) {
        return false;
    }

    if (not f.equal(std::array{12, 14, 16, 18})) {
        return false;
    }

    return true;
}
static_assert(test_zip_with2_subflow());


constexpr bool test_zip_with3_subflow()
{
    auto prod = [](auto... args) { return (args * ...); };

    auto f = flow::zip_with(prod, flow::ints(1), flow::ints(1), flow::ints(1))
        .take(5);

    f.next();

    if (not f.subflow().equal(std::array{8, 27, 64, 125})) {
        return false;
    }

    if (not f.equal(std::array{8, 27, 64, 125})) {
        return false;
    }

    return true;
}
static_assert(test_zip_with3_subflow());

constexpr bool test_zip_with2_size()
{
    auto sum = [](auto... args) { return (args + ...); };
    auto f = flow::zip_with(sum, flow::ints(), flow::of(1, 2, 3));

    [[maybe_unused]] auto inf = flow::zip_with(sum, flow::ints(), flow::ints());

    static_assert(flow::is_infinite_flow<decltype(inf)>);

    return f.size() == 3;
}
static_assert(test_zip_with2_size());

constexpr bool test_zip_with3_size()
{
    auto sum = [](auto... args) { return (args + ...); };
    auto f = flow::zip_with(sum, flow::ints(), flow::of(1, 2, 3), flow::ints());

    [[maybe_unused]] auto inf = flow::zip_with(sum, flow::ints(), flow::ints(), flow::ints());

    static_assert(flow::is_infinite_flow<decltype(inf)>);

    return f.size() == 3;
}
static_assert(test_zip_with3_size());

TEST_CASE("zip_with", "[flow.zip_with]")
{
    REQUIRE(test_zip_with2());
    REQUIRE(test_zip_with3());
    REQUIRE(test_zip_with2_subflow());
    REQUIRE(test_zip_with3_subflow());
    REQUIRE(test_zip_with2_size());
    REQUIRE(test_zip_with3_size());

    std::vector numbers = {0, 1, 2, 3};
    std::string letters = "abcd";

    auto out = flow::zip_with([](int i, char c) {
        return flow::of(c).cycle().take(i);
    }, numbers, letters).flatten().to_string();

    REQUIRE(out == "bccddd");
}

}

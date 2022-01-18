
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

#include <optional>

namespace {

constexpr bool test_deref()
{
    int a = 1, c = 3;

    // Test dereffing pointers
    {
        auto f = flow::of(&a, (int*) nullptr, &c).deref();

        if (not f.equal(flow::of(1, 3))) {
            return false;
        }
    }

    // Test dereffing maybes
    {
        std::array<flow::maybe<int&>, 3> arr{a, {}, c};

        auto f = flow::deref(arr);

        bool success =
                &f.next().value() == &a  &&
                &f.next().value() == &c &&
                not f.next().has_value();

        if (!success) {
            return false;
        }
    }

    return true;
}
static_assert(test_deref());

constexpr bool test_deref_optional()
{
    auto arr = std::array<std::optional<int>, 3>{1, {}, 3};

    auto f = flow::deref(arr);

    return f.next().value() == 1 &&
           f.next().value() == 3 &&
           not f.next().has_value();
}
// FIXME: GCC doesn't like this for some reason (but Clang is fine)
#if !COMPILER_IS_GCC
static_assert(test_deref_optional());
#endif

TEST_CASE("flow.deref", "[flow.deref]")
{
    REQUIRE(test_deref());
    REQUIRE(test_deref_optional());

    // Test dereffing unique_ptrs
    {
        auto ptrs = std::array{
            std::make_unique<int>(1),
            std::unique_ptr<int>(),
            std::make_unique<int>(3)
        };

        REQUIRE(flow::deref(ptrs).equal(flow::of(1, 3)));
    }

    // Test dereffing shared pointers
    {
        auto ptrs = std::array{
            std::make_shared<int>(1),
            std::shared_ptr<int>(),
            std::make_shared<int>(3)
        };

        REQUIRE(flow::deref(ptrs).equal(flow::of(1, 3)));
    }
}

}
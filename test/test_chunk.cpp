
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

template <typename T, size_t N>
struct carray : std::array<T, N> {
    friend constexpr bool operator==(const carray& lhs, const carray& rhs)
    {
        for (size_t i = 0; i < N; i++) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    }

    friend constexpr bool operator!=(const carray& lhs, const carray& rhs)
    {
        return !(lhs == rhs);
    }
};

template <typename T, typename... U>
carray(T, U...) -> carray<T, 1 + sizeof...(U)>;

constexpr bool test_chunk()
{
    const carray arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    if (flow::from(arr).chunk(5).count() != 2) {
        return false;
    }

    if (flow::from(arr).chunk(3).count() != 4) {
        return false;
    }

    if (flow::of(1, 2, 3).chunk(5).count() != 1) {
        return false;
    }

    // Test moderately complicated pipeline
    {
        const bool b = flow::of(1, 1, 2, -2, 6, 0, 3, 1)
                           .chunk(3)
                           .map([](auto f) { return std::move(f).sum(); })
                           .all(flow::pred::eq(4));

        if (!b) {
            return false;
        }
    }

    // Test oversized chunk
    {
        carray<int, 10> out{};
        flow::from(arr).chunk(100).for_each(
            [&out](auto f) { return std::move(f).output_to(out.begin()); });

        if (arr != out) {
            return false;
        }
    }

    // Test round-tripping through chunk and flatten
    {
        auto f = flow::chunk(arr,3).flatten();
        int counter = 0;
        FLOW_FOR(int const& i, f)
        {
            if (&i != arr.data() + counter) {
                return false;
            }
            ++counter;
        }
    }

    return true;
}
static_assert(test_chunk());

TEST_CASE("chunk()", "[flow.chunk]")
{
    REQUIRE(test_chunk());

    // Constructing a vector of vectors
    {
        const auto vecs =
            flow::iota(1)
                .take(10)
                .chunk(3)
                .map([](auto f) { return std::move(f).to_vector(); })
                .to_vector();

        REQUIRE(vecs.size() == 4);
        REQUIRE((vecs[0] == std::vector{1, 2, 3}));
        REQUIRE((vecs[1] == std::vector{4, 5, 6}));
        REQUIRE((vecs[2] == std::vector{7, 8, 9}));
        REQUIRE((vecs[3] == std::vector{10}));
    }


}

}

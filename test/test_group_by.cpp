
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

namespace {

constexpr bool test_group_by() {

    const std::array arr{1, 3, -2, -2, 1, 0, 1, 2};

    // An example from the Rust Itertools docs
    {

        bool b = flow::from(arr)
                     .group_by(not flow::pred::negative)
                     .map([](auto f) { return std::move(f).sum(); })
                     .map([](int i) { return i < 0 ? -i : i; })
                     .all(flow::pred::eq(4));

        if (!b) {
            return false;
        }
    }

    // Test round-tripping through group_by() and flatten()
    {
        auto f = flow::group_by(arr, !flow::pred::negative).flatten();

        int counter = 0;
        FLOW_FOR(int const& i, f) {
            if (&i != arr.data() + counter) {
                return false;
            }
            ++counter;
        }
    }

    // Test not consuming inner groups
    {
        const auto cnt = flow::from(arr).group_by(not flow::pred::negative).count();
        if (cnt != 3) {
            return false;
        }
    }

    // Test partially consuming inner groups
    {
        const auto want = std::array{1, -2, 1};
        std::array<int, 3> have{};

        flow::from(arr)
            .group_by(not flow::pred::negative)
            .map([](auto f) { return f.next(); })
            .unchecked_deref()
            .output_to(have.begin());

        for (int i = 0; i < 3; i++) {
            if (want[i] != have[i]) {
                return false;
            }
        }
    }

    // Test multiple group_by()s in sequence
    {
        // FIXME: this triggers a non-constexpr code path :(
/*        bool b = flow::from(arr)
                .group_by(not flow::pred::negative)
                .group_by([](auto&&) { return true; })
                .flatten()
                .map([](auto f) { return std::move(f).sum(); })
                .map([](int i) { return i < 0 ? -i : i; })
                .all(flow::pred::eq(4));

        if (!b) {
            return false;
        }*/
    }

    // Test duplicate removal using group_by
    {
        bool b = flow::c_str("aabcccccccddde")
            .group_by([](char c) { return c; })
            .map([](auto f) { return *f.next(); })
            .equal(flow::c_str("abcde"));
        if (!b) {
            return false;
        }
    }

    return true;
}
//static_assert(test_group_by());

constexpr bool test_group_by2()
{
    using P = std::pair<int, int>;

    std::array<P, 12> v =
        {
            P{1,1},
            {1,1},
            {1,2},
            {1,2},
            {1,2},
            {1,2},
            {2,2},
            {2,2},
            {2,3},
            {2,3},
            {2,3},
            {2,3}
        };

    {
        // Should be able to do group_by(&P::second), but GCC doesn't
        // seem to like PMDs in constexpr
        auto f1 = flow::from(v).group_by([](P& p) -> int& { return p.second; });
        bool success =
            f1.next().value().equal(flow::of{P{1, 1}, P{1, 1}}) &&
            f1.next().value().equal(flow::of{P{1, 2}, P{1, 2}, P{1, 2}, P{1, 2},
                                             P{2, 2}, P{2, 2}}) &&
            f1.next().value().equal(
                flow::of{P{2, 3}, P{2, 3}, P{2, 3}, P{2, 3}}) &&
            !(bool) f1.next();
        if (!success) {
            return false;
        }
    }

    {
        auto f2 = flow::from(v).group_by([](P& p) -> int& { return p.first; });
        bool success =
            f2.next().value().equal(flow::of{P{1, 1}, P{1, 1}, P{1, 2}, P{1, 2},
                                             P{1, 2}, P{1, 2}}) &&
            f2.next().value().equal(flow::of{P{2, 2}, P{2, 2}, P{2, 3}, P{2, 3},
                                             P{2, 3}, P{2, 3}}) &&
            !(bool) f2.next();
        if (!success) {
            return false;
        }
    }

    return true;
}
//static_assert(test_group_by2());

TEST_CASE("group_by", "[flow.group_by]")
{
    REQUIRE(test_group_by());
    REQUIRE(test_group_by2());

    // Test over-consuming inner flows
    {
        std::vector const in{1, 3, -2, -2, 1, 0, 1, 2};
        std::vector<int> out;

        FLOW_FOR(auto f, flow::from(in).group_by(not flow::pred::negative)) {
            FLOW_FOR(int i, f) {
                out.push_back(i);
            }
            REQUIRE_FALSE(f.next().has_value());
        }

        REQUIRE(out == in);
    }
}

}

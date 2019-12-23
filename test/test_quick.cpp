
#include <flow.hpp>

#include "catch.hpp"

// FIXME: GCC barfs if static_asserts using flow::of() are at namespace scope,
// FIXME: and I don't know why :-(
constexpr bool test_quick()
{
    static_assert(flow::of{1, 1, 1}.count() == 3);
    static_assert(flow::of{1, 1, 1}.count() == 3);
    static_assert(flow::of{1, 1, 1}.count(1) == 3);
    static_assert(flow::of(1, 2, 3, 4, 5).min().value() == 1);
    static_assert(flow::of(1, 2, 3, 4, 5).max().value() == 5);
    static_assert(flow::of(1, 2, 3).sum() == 6);
    static_assert(flow::of(1, 2, 3).product() == 6);

    auto is_one = [](int i) { return i == 1; };

    static_assert(flow::of(1, 1, 1).all(is_one));
    static_assert(!flow::of(1, 1, 2).all(is_one));
    static_assert(flow::of(2, 2, 2).none(is_one));
    static_assert(!flow::of(1, 1, 2).none(is_one));
    static_assert(flow::of(1, 2, 3).any(is_one));

    static_assert(flow::of{2, 2, 2, 1, 2, 2, 2}.contains(1));

    static_assert(flow::of{2, 1, 2, 1, 2, 1, 2}.filter(is_one).count() == 3);

    constexpr std::array arr{2, 1, 2};
    static_assert(std::addressof(flow::from(arr).find(1).value()) == arr.data() + 1);

    static_assert(flow::of{1, 1, 1}.is_sorted());
    static_assert(!flow::of{1, 2, 1}.is_sorted());

    return true;
}
static_assert(test_quick());


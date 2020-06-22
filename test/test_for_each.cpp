
#include <flow.hpp>

namespace {

constexpr bool test_for_each()
{
    int i = 0;
    auto counter = [&i] (const auto&) { ++i; };

    flow::of(1, 2, 3).for_each(counter);

    return i == 3;
}
static_assert(test_for_each());

constexpr bool test_for_each2()
{
    std::array in{1, 2, 3, 4, 5};

    struct fn {
        constexpr void operator()(int i) { total += i; }
        int total = 0;
    };

    auto f = flow::for_each(in, fn{});

    return f.total == 15;
}
static_assert(test_for_each2());

}
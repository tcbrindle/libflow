
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

}
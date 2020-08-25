
#include <flow.hpp>

#include "catch.hpp"

// This file is a bit of a mess because the regex used to add Catch tests to
// CTest doesn't look inside ifdefs, so we need to ensure the tests are always
// present (and will trivially pass) even when we don't have coro support.

namespace {

#ifdef FLOW_HAVE_COROUTINES

auto get_names() -> flow::async<std::string>
{
    co_yield "Adam";
    co_yield "Barbara";
    co_yield "Clive";
}

auto ints(int from = 0) -> flow::async<int>
{
    while (true) {
        co_yield from++;
    }
}

auto ints(int from, int to) -> flow::async<int>
{
    while (from < to) {
        co_yield from++;
    }
}

using triple = std::tuple<int, int, int>;

auto pythagorean_triples() -> flow::async<triple>
{
    FLOW_FOR(int z, ::ints(1))
    {
        FLOW_FOR(int y, ::ints(1, z))
        {
            FLOW_FOR(int x, ::ints(1, y))
            {
                if (x * x + y * y == z * z) {
                    co_yield {x, y, z};
                }
            }
        }
    }
}

#endif // FLOW_HAVE_COROUTINES

TEST_CASE("async names", "[flow.async]")
{
#ifdef FLOW_HAVE_COROUTINES
    std::vector<std::string> vec = get_names().collect();

    REQUIRE(vec.size() == 3);
    REQUIRE((vec == std::vector<std::string>{"Adam", "Barbara", "Clive"}));

    REQUIRE(get_names().sum() == "AdamBarbaraClive");
#endif
}

TEST_CASE("async ints", "[flow.async]")
{
#ifdef FLOW_HAVE_COROUTINES
    auto vec = ints().take(5).to_vector();

    REQUIRE(vec.size() == 5);
    for (std::size_t i = 0; i < vec.size(); i++) {
        REQUIRE(vec[i] == i);
    }
#endif
}

TEST_CASE("async triples", "[flow.async]")
{
#ifdef FLOW_HAVE_COROUTINES
    for (auto [x, y, z] : pythagorean_triples().take(10).to_range()) {
        REQUIRE(x * x + y * y == z * z);
    }
#endif
}

}

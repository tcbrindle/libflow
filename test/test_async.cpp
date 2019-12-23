
#include <flow.hpp>

#ifdef FLOW_HAVE_COROUTINES

#include "catch.hpp"

#include <iostream>

namespace {

auto get_names() -> flow::async<std::string> {
    co_yield "Adam";
    co_yield "Barbara";
    co_yield "Clive";
}

auto ints(int from = 0) -> flow::async<int> {
    while (true) {
        co_yield from++;
    }
}

auto ints(int from, int to) -> flow::async<int> {
    while (from < to) {
        co_yield from++;
    }
}

using triple = std::tuple<int, int, int>;

auto pythagorean_triples() -> flow::async<triple>
{
    FLOW_FOR (int z, ::ints(1)) {
        FLOW_FOR (int y, ::ints(1, z)) {
            FLOW_FOR (int x, ::ints(1, y)) {
                if (x*x + y*y == z*z) {
                    co_yield {x, y, z};
                }
            }
        }
    }
}

}

TEST_CASE("async names", "[flow.async]")
{
    std::vector<std::string> vec = get_names().to_vector();

    REQUIRE(vec.size() == 3);
    REQUIRE((vec == std::vector<std::string>{"Adam", "Barbara", "Clive"}));

    REQUIRE(get_names().sum() == "AdamBarbaraClive");
}

TEST_CASE("all the ints", "[flow.async]")
{
    auto vec = ints().take(5).to_vector();

    REQUIRE(vec.size() == 5);
    for (int i = 0; i < vec.size(); i++) {
        REQUIRE(vec[i] == i);
    }
}

TEST_CASE("Triples", "[flow.async]")
{
    for (auto [x, y, z] : pythagorean_triples().take(10).to_range()) {
        REQUIRE(x*x + y*y == z*z);
    }
}


#endif // FLOW_HAVE_COROUTINES
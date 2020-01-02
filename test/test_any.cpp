
#include <flow.hpp>

#include "catch.hpp"

#include <iostream>

namespace {

void takes_any(flow::any<int&> f) {
    auto vec = std::move(f).to_vector();
    REQUIRE((vec == std::vector{1, 2, 3}));
}

void takes_any_ref(flow::any_ref<char> f) {
    std::string str = std::move(f).to_string();
    REQUIRE(str == "abcdef");
}

}

TEST_CASE("any", "[flow.any]")
{
    takes_any(flow::of(1, 2, 3));
}

TEST_CASE("any_ref", "[flow.any_ref]")
{
    auto flow = flow::iota('a').take(6);
    takes_any_ref(flow);
}
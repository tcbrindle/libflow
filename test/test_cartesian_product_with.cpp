
#include "catch.hpp"

#include <flow.hpp>

#include <iostream>

namespace {

TEST_CASE("Cartesian product with")
{
    std::vector<int> vec{1, 2, 3};

    auto c = flow::cartesian_product_with([](int i, int j, int k) {
        std::cout << i << ' ' << j << ' ' << k << '\n';
        return 0;
    }, flow::reverse(vec), vec, flow::map(vec, [](int i) { return i * 2; })).count();

    REQUIRE(c == 3 * 3 * 3);
}

}
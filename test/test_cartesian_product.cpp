
#include "catch.hpp"

#include <flow.hpp>
#include <iostream>

TEST_CASE("cartesian_product()")
{
    auto const vec1 = std::vector{1, 2, 3};
    auto const vec2 = std::vector{1.f, 2.f, 3.f};
    auto plus_point_1 = [](auto x) { return x + 0.1; };

    auto prod = flow::cartesian_product(vec1, vec2, flow::map(vec2, plus_point_1));

    static_assert(std::is_same_v<flow::item_t<decltype(prod)>, std::tuple<int const&, float const&, double>>);

    std::move(prod).for_each([](auto p) {
        auto const& [x, y, z] = p;
        std::cout << x << ' ' << y << ' ' << z << '\n';
    });
}
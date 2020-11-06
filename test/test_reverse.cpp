
#include <flow.hpp>

#include "catch.hpp"

namespace {

constexpr bool test()
{
    {
        auto in = std::array{1, 2, 3, 4, 5};
        auto out = std::array{5, 4, 3, 2, 1};

        if (!flow::from(in).reverse().equal(out)) {
            return false;
        }
    }

    {
        auto in = std::array{1, 2, 3, 4, 5};
        auto out = std::array{5, 4, 3, 2, 1};

        if (!flow::reverse(in).equal(out)) {
            return false;
        }
    }

    return true;
}
static_assert(test());

TEST_CASE("reverse() works for vectors")
{
    auto vec = std::vector<int>{1, 2, 3, 4, 5};

    REQUIRE((flow::from(vec).reverse().to_vector() ==
             std::vector<int>{5, 4, 3, 2, 1}));
}

}
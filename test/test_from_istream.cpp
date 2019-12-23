
#include <flow.hpp>

#include "catch.hpp"

#include <sstream>

TEST_CASE("from_istream()", "[flow.from_istream]")
{
    std::istringstream iss{"1 2 3 4 5"};
    auto vec = flow::from_istream<int>(iss).to_vector();
    REQUIRE((vec == std::vector<int>{1, 2, 3, 4, 5}));
}

// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <sstream>

TEST_CASE("from_istream()", "[flow.from_istream]")
{
    std::istringstream iss{"1 2 3 4 5"};
    auto vec = flow::from_istream<int>(iss).to_vector();
    REQUIRE((vec == std::vector<int>{1, 2, 3, 4, 5}));
}
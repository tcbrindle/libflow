
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <sstream>

namespace {

TEST_CASE("Write to", "[flow.write_to]")
{
    std::ostringstream oss;

    flow::of{1, 2, 3, 4, 5}.filter(flow::pred::odd).write_to(oss);

    REQUIRE(oss.str() == "1, 3, 5");
}

TEST_CASE("Write to wide stream", "[flow.write_to]")
{
    std::wostringstream woah;

    flow::write_to(std::vector{1, 2, 3, 4, 5}, woah, '0');

    REQUIRE(woah.str() == L"102030405");
}


}
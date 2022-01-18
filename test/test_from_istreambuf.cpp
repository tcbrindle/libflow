
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include "catch.hpp"

#include <charconv>
#include <sstream>

namespace {

TEST_CASE("from_istreambuf basic", "[flow.from_istreambuf]")
{
    std::istringstream iss{"Hello World"};

    REQUIRE(flow::from_istreambuf(iss).map(::toupper).to_string() == "HELLO WORLD");

    std::istringstream names{"Adam Barbara Charlie Danielle Edward"};

    const auto vec = flow::from_istreambuf(names)
                   // HACKHACKHACK
                   .apply([](auto&& f) { return flow::from(std::move(f).to_vector()); })
                   .split(' ')
                   .map([](auto f) { return std::move(f).to_string(); })
                   .to_vector();

    REQUIRE((vec == std::vector<std::string>{
                       "Adam", "Barbara", "Charlie", "Danielle", "Edward"
                   }));
}

std::array<uint8_t, 6> parse_mac_address(std::istringstream& str)
{
    auto f = flow::from_istreambuf(str)
            // HACKHACKHACK
            .apply([](auto&& f) { return flow::from(std::move(f).to_vector()); })
            .split(':')
            .map([](auto f) {
                auto s = std::move(f).to_string();
                uint8_t c;
                std::from_chars(s.data(), s.data() + s.length(), c, 16);
                return c;
            }).zip(flow::ints());

    std::array<uint8_t, 6> out{};

    FLOW_FOR(auto p, f) {
        auto [c, idx] = p;
        out[idx] = c;
    }

    return out;
}

TEST_CASE("from_istreambuf basic parsing", "[flow.from_istreambuf]")
{
    std::istringstream source{"aa::bb::cc::dd::ee::ff"};

    const auto mac = parse_mac_address(source);

    REQUIRE(mac == std::array<uint8_t, 6>{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF});
}

TEST_CASE("from_istreambuf with wide characters", "[flow.from_istreambuf]")
{
    std::wistringstream iss{L"Hello World"};

    REQUIRE(flow::from_istreambuf(iss).equal(flow::c_str(L"Hello World")));
}

}
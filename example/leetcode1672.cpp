
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

// You are given an m x n integer grid accounts where accounts[i][j] is the
// amount of money the ith customer has in the jth bank. Return the wealth that
// the richest customer has.
//
// A customer's wealth is the amount of money they have in all their bank
// accounts. The richest customer is the customer that has the maximum wealth.

// https://leetcode.com/problems/richest-customer-wealth/

constexpr auto max_wealth = [](auto const& accounts) {
    return flow::map(accounts, flow::sum).max().value();
};

using accounts_t = std::vector<std::vector<int>>;

int main()
{
    {
        const auto acc = accounts_t{{1, 2, 3}, {3, 2, 1}};
        assert(max_wealth(acc) == 6);
    }

    {
        const auto acc = accounts_t{{1, 5}, {7, 3}, {3, 5}};
        assert(max_wealth(acc) == 10);
    }

    {
        const auto acc = accounts_t{{2, 8, 7}, {7, 1, 3}, {1, 9, 5}};
        assert(max_wealth(acc) == 17);
    }
}
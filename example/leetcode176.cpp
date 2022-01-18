
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include <iostream>

/*
 * Taken from
 * LeetCode 176 Problem 1 - Count Negative Numbers in a Sorted Matrix
 * YouTube video by Conor Hoekstra (code_report)
 * https://youtu.be/pDbDtGn1PXk
 */

int main()
{
    // The problem from the video...
    const std::vector<std::vector<int>> grid = {
        { -2, -1,  0 },
        { -1,  1,  3 },
        { -1,  2,  4 }
    };

    const auto num = flow::flatten(grid).count_if(flow::pred::negative);

    std::cout << "There are " << num << " negative numbers in the grid\n";

    // or we can be really fancy and do it all at compile-time
    constexpr std::array grid2 = {
        std::array{-2, -1, 0},
        std::array{-1, 1, 3},
        std::array{-1, 2, 4}
    };

    static_assert(flow::flatten(grid2).count_if(flow::pred::negative) == 4);
}
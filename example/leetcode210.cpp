
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

// Taken from "1 Problem, 8 Programming Languages" YouTube video by
// Conor Hoekstra (@code_report)
// https://www.youtube.com/watch?v=zrOIQEN3Wkk

// Original Leetcode contest problem:
// https://leetcode.com/contest/weekly-contest-210/problems/maximum-nesting-depth-of-the-parentheses/

constexpr auto max_depth = [](std::string_view const input) {
    return flow::filter(input, flow::pred::in('(', ')'))
        .map([](char c) { return c == '(' ? 1 : -1; })
        .partial_sum()
        .max().value_or(0);
};

int main()
{
    {
        constexpr auto in = "(1+(2*3)+((8)/4))+1";
        static_assert(max_depth(in) == 3);
    }

    {
        constexpr auto in = "(1)+((2))+(((3)))";
        static_assert(max_depth(in) == 3);
    }

    {
        constexpr auto in = "1+(2*3)/(2-1)";
        static_assert(max_depth(in) == 1);
    }

    {
        constexpr auto in = "1";
        static_assert(max_depth(in) == 0);
    }
}

// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include <iostream>

/*
 * Taken from
 * LeetCode 168 Problem 1 - Find Numbers with Even Number of Digits
 * YouTube video by Conor Hoekstra (code_report)
 * https://youtu.be/gZLZPrgqw2A
 */

static auto find_number(std::vector<int>& nums)
{
    return flow::from(nums)
                 .map([](int i) { return std::to_string(i).size(); })
                 .count_if(flow::pred::even);
}


int main()
{
    std::vector numbers = {12, 345, 2, 6, 7896};

    std::cout << "There were " << find_number(numbers)
              << " integers with an even number of digits\n";
}
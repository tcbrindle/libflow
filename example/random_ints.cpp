
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include <iostream>
#include <random>

// An example of writing a custom flow, in this case one that generates
// an endless sequence of random ints within a given interval

struct random_ints : flow::flow_base<random_ints> {

    random_ints(int from, int to)
        : dist(from, to)
    {}

    auto next() -> flow::maybe<int>
    {
        return {dist(gen)};
    }

private:
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<int> dist;
};

int main()
{
    random_ints(0, 100)
        .take(10)
        .write_to(std::cout, "\n");
}
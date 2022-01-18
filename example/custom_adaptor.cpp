
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include <algorithm>
#include <iostream>

/*
 * libflow's powerful apply() method makes it easy to implement custom adaptors
 * and use them with the postfix "dot" notation.
 *
 * This example shows how we can use this function to provide a `sort` operation,
 * by copying a flow into a vector, sorting the vector, and then returning a
 * flow so that we can continue chaining.
 */

auto sort = [](auto flow) {
    auto vec = std::move(flow).to_vector(); // collect the flow into a vector
    std::sort(vec.begin(), vec.end());     // sort it
    return flow::from(std::move(vec));      // convert it into a new flow
};

int main()
{
    flow::of{5, 4, 3, 2, 1}
        .filter(flow::pred::odd)
        .apply(sort) // apply our sort operation and continue chaining
        .map([](int i) { return i * i; })
        .write_to(std::cout);
}
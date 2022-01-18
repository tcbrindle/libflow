
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include <algorithm>
#include <iostream>

/*
 * "That's a rotate!"
 *
 * In this example we'll again use apply() to implement a custom adaptor as
 * part of a flow pipeline. This time we'll write a slightly more complicated
 * adaptor which takes a parameter.
 */

auto rotate_by =  [] (auto flow, flow::dist_t places) {
    auto vec = std::move(flow).to_vector();

    const auto sz = static_cast<flow::dist_t>(vec.size());

    auto d = places % sz;
    d = d < 0 ? d += sz : d;

    std::rotate(vec.begin(), vec.begin() + d, vec.end());
    return flow::from(std::move(vec));
};

int main()
{
    flow::ints(1)
        .take(10)
        .apply(rotate_by, -3)
        .map([](int i) { return i * i; })
        .write_to(std::cout);
}

// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

constexpr bool test_empty()
{
    using E = flow::empty<int>;

    auto is_one  = [](int i) { return i == 1; };

    static_assert(!E{}.next());
    static_assert(!E{}.advance(1));
    static_assert(E{}.count() == 0);
    static_assert(E{}.count(1) == 0);
    static_assert(!E{}.contains(1));
    static_assert(!E{}.min());
    static_assert(!E{}.max());
    static_assert(E{}.sum() == 0);
    static_assert(E{}.product() == 1); // NB
    static_assert(E{}.all(is_one));
    static_assert(E{}.none(is_one));
    static_assert(!E{}.any(is_one));
    static_assert(E{}.filter(is_one).count() == 0);


    return true;
}
static_assert(test_empty());
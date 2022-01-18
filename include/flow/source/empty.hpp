
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_EMPTY_HPP_INCLUDED
#define FLOW_SOURCE_EMPTY_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

template <typename T>
struct empty : flow_base<empty<T>> {

    explicit constexpr empty() = default;

    static constexpr auto next() -> maybe<T> { return {}; }

    static constexpr auto size() -> dist_t { return 0; }
};

}

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_OF_HPP_INCLUDED
#define FLOW_SOURCE_OF_HPP_INCLUDED

#include <flow/source/from.hpp>

#include <array>

namespace flow {

namespace detail {

template <typename T, std::size_t N>
using array_flow = decltype(flow::from(std::declval<std::array<T, N>>()));

}

template <typename T, std::size_t N>
struct of : flow_base<of<T, N>> {
    template <typename... Args>
    constexpr explicit of(Args&&... args)
        : arr_(flow::from(std::array<T, N>{FLOW_FWD(args)...}))
    {}

    constexpr auto next() -> maybe<T&> {
        return arr_.next();
    }

    constexpr auto next_back() -> maybe<T&> {
        return arr_.next_back();
    }

    constexpr auto subflow() &
    {
        return arr_.subflow();
    }

    constexpr auto size() const -> dist_t
    {
        return arr_.size();
    }

private:
    detail::array_flow<T, N> arr_;
};

template <typename T, typename... U>
of(T, U...) -> of<T, 1 + sizeof...(U)>;

}

#endif

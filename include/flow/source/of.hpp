
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
struct of : detail::array_flow<T, N> {
    template <typename... Args>
    constexpr explicit of(Args&&... args)
        : detail::array_flow<T, N>(flow::from(std::array<T, N>{FLOW_FWD(args)...}))
    {}
};

template <typename T, typename... U>
of(T, U...) -> of<T, 1 + sizeof...(U)>;

template <typename T>
struct empty : flow_base<empty<T>> {

    explicit constexpr empty() = default;

    constexpr auto next() -> maybe<T> { return {}; }
};

}

#endif

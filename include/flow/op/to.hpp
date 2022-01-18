
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_TO_HPP_INCLUDED
#define FLOW_OP_TO_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Void, template <typename...> typename C, typename... Args>
inline constexpr bool is_ctad_constructible_v = false;

template <template <typename...> typename C, typename... Args>
inline constexpr bool is_ctad_constructible_v<
    std::void_t<decltype(C(std::declval<Args>()...))>, C, Args...> = true;

}

// These need to be function templates so that the user can supply the template
// arg, and we can overload on both type template parameters and
// template template params
template <typename C, typename Flowable>
constexpr auto to(Flowable&& flowable) -> C
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template to<C>();
}

template <template <typename...> typename C, typename Flowable>
constexpr auto to(Flowable&& flowable)
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template to<C>();
}

template <typename Flowable>
auto to_vector(Flowable&& flowable) -> std::vector<flow_value_t<Flowable>>
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to_vector() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).to_vector();
}

template <typename T, typename Flowable>
auto to_vector(Flowable&& flowable) -> std::vector<T>
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to_vector() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template to_vector<T>();
}

template <typename Flowable>
auto to_string(Flowable&& flowable) -> std::string
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to_string() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).to_string();
}

template <typename D>
template <typename C>
constexpr auto flow_base<D>::to() && -> C
{
    if constexpr (std::is_constructible_v<C, D&&>) {
        return C(consume());
    } else {
        auto rng = consume().to_range();
        static_assert(std::is_constructible_v<C, decltype(rng.begin()),
                                              decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }
}

template <typename D>
template <template <typename...> typename C>
constexpr auto flow_base<D>::to() &&
{
    if constexpr (detail::is_ctad_constructible_v<void, C, D&&>) {
        return C(consume());
    } else {
        auto rng = consume().to_range();
        static_assert(detail::is_ctad_constructible_v<void, C,
                      decltype(rng.begin()), decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }
}

template <typename D>
template <typename T>
auto flow_base<D>::to_vector() && -> std::vector<T>
{
    return consume().template to<std::vector<T>>();
}

template <typename D>
auto flow_base<D>::to_vector() &&
{
    return consume().template to<std::vector<value_t<D>>>();
}

template <typename D>
auto flow_base<D>::to_string() && -> std::string
{
    return consume().template to<std::string>();
}

}

#endif


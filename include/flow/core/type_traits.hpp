
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED
#define FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace flow {

template <typename T>
using remove_cvref_t = std::remove_const_t<std::remove_reference_t<T>>;

template <typename T>
using remove_rref_t = std::conditional_t<
    std::is_rvalue_reference_v<T>, std::remove_reference_t<T>, T>;

template <typename F>
using next_t = decltype(std::declval<F>().next());

template <typename F>
using item_t = typename next_t<F>::value_type;

template <typename F>
using value_t = remove_cvref_t<item_t<F>>;

template <typename F>
using subflow_t = decltype(std::declval<F&>().subflow());

using dist_t = std::make_signed_t<std::size_t>;

template <typename Derived>
struct flow_base;

template <typename>
class maybe;

namespace detail {

template <typename>
inline constexpr bool is_maybe = false;

template <typename T>
inline constexpr bool is_maybe<maybe<T>> = true;

template <typename, typename = void>
inline constexpr bool has_next = false;

template <typename T>
inline constexpr bool has_next<T, std::enable_if_t<is_maybe<next_t<T>>>> = true;

} // namespace detail

template <typename I, typename R = std::remove_reference_t<I>>
inline constexpr bool is_flow
    = std::is_move_constructible_v<R> &&
      std::is_base_of_v<flow_base<R>, R> &&
      std::is_convertible_v<std::add_lvalue_reference_t<I>, flow_base<R>&> &&
      detail::has_next<I>;

namespace detail {

template <typename, typename = void>
inline constexpr bool has_subflow = false;

template <typename T>
inline constexpr bool has_subflow<
    T, std::enable_if_t<is_flow<subflow_t<T>>>> = true;

} // namespace detail

template <typename F>
inline constexpr bool is_multipass_flow = is_flow<F> && detail::has_subflow<F>;

namespace detail {

template <typename, typename = void>
inline constexpr bool has_size = false;

template <typename T>
inline constexpr bool has_size<
    T, std::void_t<decltype(std::declval<T const&>().size())>> = true;

template <typename, typename = void>
inline constexpr bool is_infinite = false;

template <typename T>
inline constexpr bool is_infinite<T, std::enable_if_t<T::is_infinite>> = true;

}

template <typename F>
inline constexpr bool is_sized_flow = is_flow<F> && detail::has_size<F>;

template <typename F>
inline constexpr bool is_infinite_flow = is_flow<F> && detail::is_infinite<F>;

namespace detail {

template <typename, typename = void>
inline constexpr bool has_next_back = false;

template <typename T>
inline constexpr bool has_next_back<T, std::enable_if_t<
    std::is_same_v<next_t<T>, decltype(std::declval<T>().next_back())>>> = true;

}

template <typename F>
inline constexpr bool is_reversible_flow = is_flow<F> && detail::has_next_back<F>;

} // namespace flow

#endif

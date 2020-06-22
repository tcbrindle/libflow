
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

using dist_t = std::make_signed_t<std::size_t>;

template <typename T>
class maybe;

template <typename Derived>
struct flow_base;

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
    = std::is_base_of_v<flow_base<R>, R> &&
      std::is_convertible_v<std::add_lvalue_reference_t<I>, flow_base<R>&> &&
      detail::has_next<I>;

} // namespace flow

#endif

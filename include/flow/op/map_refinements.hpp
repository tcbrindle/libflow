
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_MAP_REFINEMENTS_HPP_INCLUDED
#define FLOW_OP_MAP_REFINEMENTS_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

//
// These are all small adaptors which end up calling map(<some lambda>)
//

namespace flow {

// as()

namespace detail {

template <typename T>
struct as_op {
    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable) const
    {
        static_assert(is_flowable<decltype(flowable)>,
                      "Argument to flow::as() must be a Flowable type");
        return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template as<T>();
    }
};

}

template <typename T>
inline constexpr auto as = detail::as_op<T>{};

template <typename D>
template <typename T>
constexpr auto flow_base<D>::as() &&
{
    static_assert(!std::is_void_v<T>,
                  "as() cannot be called with a void type");

    return consume().map([](auto&& val) -> T {
        return static_cast<T>(FLOW_FWD(val));
    });
}

// unchecked_deref()

inline constexpr auto unchecked_deref = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::unchecked_deref() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).unchecked_deref();
};

template <typename D>
constexpr auto flow_base<D>::unchecked_deref() &&
{
    auto deref = [](auto&& val) -> decltype(*FLOW_FWD(val)) {
        return *FLOW_FWD(val);
    };
    static_assert(std::is_invocable_v<decltype(deref), item_t<D>>,
                  "Flow's item type is not dereferenceable with unary operator*");
    static_assert(!std::is_void_v<decltype(*std::declval<item_t<D>>())>,
                  "Flow's item type dereferences to void");
    return consume().map(std::move(deref));
}

// copy()

inline constexpr auto copy = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::copy() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).copy();
};

template <typename D>
constexpr auto flow_base<D>::copy() && -> decltype(auto)
{
    if constexpr (std::is_lvalue_reference_v<item_t<D>>) {
        return consume().template as<value_t<D>>();
    } else {
        return consume();
    }
}

// move()

inline constexpr auto move = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::move() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).move();
};

template <typename D>
constexpr auto flow_base<D>::move() && -> decltype(auto)
{
    if constexpr (std::is_lvalue_reference_v<item_t<D>>) {
        return consume().map([](auto& item) { return std::move(item); });
    } else {
        return consume();
    }
}

// as_const()

inline constexpr auto as_const = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::as_const() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).as_const();
};

template <typename D>
constexpr auto flow_base<D>::as_const() && -> decltype(auto)
{
    using I = item_t<D>;

    if constexpr (std::is_lvalue_reference_v<I> &&
                  !std::is_const_v<std::remove_reference_t<I>>) {
        return consume().template as<std::remove_reference_t<I> const&>();
    } else {
        return consume();
    }
}

// elements

namespace detail {

template <std::size_t N>
struct elements_op {
    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable) const
    {
        static_assert(is_flowable<decltype(flowable)>,
                      "Argument to flow::elements() must be a Flowable type");
        return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template elements<N>();
    }
};

// This should be a lambda inside elements<N>(), but MSVC doesn't like it at all
template <std::size_t N>
struct tuple_getter {
    template <typename Tuple>
    constexpr auto operator()(Tuple&& tup) const
        -> remove_rref_t<decltype(std::get<N>(FLOW_FWD(tup)))>
    {
        return std::get<N>(FLOW_FWD(tup));
    }
};

} // namespace detail

template <std::size_t N>
inline constexpr auto elements = detail::elements_op<N>{};

template <typename D>
template <std::size_t N>
constexpr auto flow_base<D>::elements() &&
{
    static_assert(std::is_invocable_v<detail::tuple_getter<N>&, item_t<D>>,
                  "Flow's item type is not tuple-like");

    return consume().map(detail::tuple_getter<N>{});
}

// keys

inline constexpr auto keys = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::keys() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).keys();
};

template <typename D>
constexpr auto flow_base<D>::keys() &&
{
    return consume().template elements<0>();
}

// values

inline constexpr auto values = [](auto&& flowable) {
  static_assert(is_flowable<decltype(flowable)>,
                "Argument to flow::values() must be a Flowable type");
  return FLOW_COPY(flow::from(FLOW_FWD(flowable))).values();
};

template <typename D>
constexpr auto flow_base<D>::values() &&
{
    return consume().template elements<1>();
}

}

#endif


#ifndef FLOW_OP_MAP_REFINEMENTS_HPP_INCLUDED
#define FLOW_OP_MAP_REFINEMENTS_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

//
// These are all small adaptors which end up calling map(<some lambda>)
//

namespace flow {

// as()

template <typename T>
inline constexpr auto as = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::as() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template as<T>();
};

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

// deref()

inline constexpr auto deref = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::deref() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).deref();
};

template <typename D>
constexpr auto flow_base<D>::deref() &&
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
        return consume().template as<std::remove_reference_t<item_t<D>>&&>();
    } else {
        return consume();
    }
}

}

#endif

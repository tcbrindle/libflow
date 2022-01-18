
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_COUNT_HPP_INCLUDED
#define FLOW_OP_COUNT_HPP_INCLUDED

#include <flow/op/count_if.hpp>

namespace flow {

namespace detail {

struct count_op {

    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable) const
    {
        static_assert(is_flow<Flowable>,
            "Argument to flow::count() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).count();
    }

    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& value, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::count() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).count(value, std::move(cmp));
    }

};

} // namespace detail

inline constexpr auto count = detail::count_op{};

template <typename Derived>
constexpr auto flow_base<Derived>::count() -> dist_t
{
    return consume().count_if([](auto const& /*unused*/) {
        return true;
    });
}

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::count(const T& item, Cmp cmp) -> dist_t
{
    using const_item_t = std::add_lvalue_reference_t<
        std::add_const_t<std::remove_reference_t<item_t<Derived>>>>;
    static_assert(std::is_invocable_r_v<bool, Cmp&, const T&, const_item_t>,
        "Incompatible comparator used with count()");

    return consume().count_if([&item, &cmp] (auto const& val) {
        return invoke(cmp, item, val);
    });
}

}

#endif

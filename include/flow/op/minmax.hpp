
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_MINMAX_HPP_INCLUDED
#define FLOW_OP_MINMAX_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct min_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::min() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).min(std::move(cmp));
    }
};

struct max_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::max() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).max(std::move(cmp));
    }
};

struct minmax_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::minmax() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).minmax(std::move(cmp));
    }
};

}

inline constexpr auto min = detail::min_op{};
inline constexpr auto max = detail::max_op{};
inline constexpr auto minmax = detail::minmax_op{};

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::min(Cmp cmp)
{
    return derived().fold_first([&cmp](auto min, auto&& item) {
        return invoke(cmp, item, min) ? FLOW_FWD(item) : std::move(min);
    });
}

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::max(Cmp cmp)
{
    return derived().fold_first([&cmp](auto max, auto&& item) {
        return !invoke(cmp, item, max) ? FLOW_FWD(item) : std::move(max);
    });
}

template <typename V>
struct minmax_result {
    V min;
    V max;

    friend constexpr bool operator==(const minmax_result& lhs, const minmax_result& rhs)
    {
        return lhs.min == rhs.min && lhs.max == rhs.max;
    }

    friend constexpr bool operator!=(const minmax_result& lhs, const minmax_result& rhs)
    {
        return !(lhs == rhs);
    }
};

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::minmax(Cmp cmp)
{
    return derived().next().map([this, &cmp] (auto&& init) {
        return derived().fold([&cmp](auto mm, auto&& item) -> minmax_result<value_t<D>> {

            if (invoke(cmp, item, mm.min)) {
                mm.min = item;
            }

            if (!invoke(cmp, item, mm.max)) {
                mm.max = FLOW_FWD(item);
            }

            return mm;
        }, minmax_result<value_t<D>>{init, FLOW_FWD(init)});
    });
}

}

#endif

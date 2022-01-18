
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_ALL_ANY_NONE_HPP_INCLUDED
#define FLOW_OP_ALL_ANY_NONE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr auto all = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::all() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).all(std::move(pred));
};

inline constexpr auto any = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::any() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).any(std::move(pred));
};

inline constexpr auto none = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::none() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).none(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::all(Pred pred) -> bool
{
    static_assert(std::is_invocable_r_v<bool, Pred&, item_t<D>>,
                  "Predicate must be callable with the Flow's item_type,"
                   " and must return bool");
    return derived().try_fold([&pred](bool /*unused*/, auto&& m) {
      return invoke(pred, *FLOW_FWD(m));
    }, true);
}

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::any(Pred pred) -> bool
{
    return !derived().none(std::move(pred));
}

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::none(Pred pred) -> bool
{
    return derived().all(pred::not_(std::move(pred)));
}

}

#endif

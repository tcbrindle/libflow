
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_INTERLEAVE_HPP_INCLUDED
#define FLOW_OP_INTERLEAVE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow1, typename Flow2>
struct interleave_adaptor : flow_base<interleave_adaptor<Flow1, Flow2>> {

    constexpr interleave_adaptor(Flow1&& flow1, Flow2&& flow2)
        : flow1_(std::move(flow1)),
          flow2_(std::move(flow2))
    {}

    constexpr auto next() -> next_t<Flow1>
    {
        auto item = first_ ? flow1_.next() : flow2_.next();
        first_ = !first_;
        return item;
    }

    template <typename F1 = Flow1, typename F2 = Flow2>
    constexpr auto subflow() & -> interleave_adaptor<subflow_t<F1>, subflow_t<F2>>
    {
        auto i = interleave_adaptor<subflow_t<F1>, subflow_t<F2>>(flow1_.subflow(), flow2_.subflow());
        i.first_ = first_;
        return i;
    }

    template <typename F1 = Flow1, typename F2 = Flow2,
              typename = std::enable_if_t<is_sized_flow<F1> && is_sized_flow<F2>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return flow1_.size() + flow2_.size();
    }

private:
    template <typename, typename>
    friend struct interleave_adaptor;

    Flow1 flow1_;
    Flow2 flow2_;
    bool first_done_ = false;
    bool second_done_ = false;
    bool first_ = true;
};

}

inline constexpr auto interleave = [](auto&& flowable1, auto&& flowable2)
{
    static_assert(is_flowable<decltype(flowable1)>,
                  "Argument to flow::interleave() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable1))).interleave(FLOW_FWD(flowable2));
};

template <typename D>
template <typename Flowable>
constexpr auto flow_base<D>::interleave(Flowable&& with) &&
{
    static_assert(is_flowable<Flowable>,
        "Argument to interleave() must be a Flowable type");
    static_assert(std::is_same_v<item_t<D>, flow_item_t<Flowable>>,
        "Flows used with interleave() must have the exact same item type");
    return detail::interleave_adaptor<D, std::decay_t<flow_t<Flowable>>>(
        consume(), FLOW_COPY(flow::from(FLOW_FWD(with))));
}

}

#endif
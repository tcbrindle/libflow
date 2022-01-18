
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_DROP_HPP_INCLUDED
#define FLOW_OP_DROP_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow>
struct drop_adaptor : flow_base<drop_adaptor<Flow>>
{
    static constexpr bool is_infinite = is_infinite_flow<Flow>;

    constexpr drop_adaptor(Flow&& flow, dist_t count)
        : flow_(std::move(flow)),
          count_(count)
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (count_ > 0) {
            (void) flow_.advance(count_);
            count_ = 0;
        }
        return flow_.next();
    }

    constexpr auto advance(dist_t dist) -> next_t<Flow>
    {
        if (count_ > 0) {
            (void) flow_.advance(count_);
            count_ = 0;
        }
        return flow_.advance(dist);
    }

    template <typename F = Flow>
    constexpr auto next_back() -> std::enable_if_t<
        is_reversible_flow<F> && is_sized_flow<F>, next_t<Flow>>
    {
        if (size() > 0) {
            return flow_.next_back();
        }
        return {};
    }

    template <typename F = Flow>
    constexpr auto size() const -> std::enable_if_t<is_sized_flow<F>, dist_t>
    {
        return max(flow_.size() - count_, dist_t{0});
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> drop_adaptor<subflow_t<F>>
    {
        return {flow_.subflow(), count_};
    }

private:
    Flow flow_;
    dist_t count_;
};

}

inline constexpr auto drop = [](auto&& flowable, dist_t count)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::drop() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).drop(count);
};

template <typename D>
constexpr auto flow_base<D>::drop(dist_t count) &&
{
    assert(count >= 0 && "Cannot drop a negative number of items");
    return detail::drop_adaptor<D>(consume(), count);
}

}

#endif

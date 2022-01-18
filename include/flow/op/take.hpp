
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_TAKE_HPP_INCLUDED
#define FLOW_OP_TAKE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow>
struct take_adaptor : flow_base<take_adaptor<Flow>> {

    constexpr take_adaptor(Flow&& flow, dist_t count)
        : flow_(std::move(flow)),
          count_(count)
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (count_ > 0) {
            --count_;
            return flow_.next();
        }
        return {};
    }

    constexpr auto advance(dist_t dist) -> next_t<Flow>
    {
        if (count_ >= dist) {
            count_ -= dist;
            return flow_.advance(dist);
        }
        return {};
    }

    template <bool B = is_reversible_flow<Flow> && is_sized_flow<Flow>>
    constexpr auto next_back() -> std::enable_if_t<B, next_t<Flow>>
    {
        if (size() > 0) {
            return flow_.next_back();
        }
        return {};
    }

    template <typename F = Flow,
              typename = std::enable_if_t<is_sized_flow<F> || is_infinite_flow<F>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        if constexpr (is_infinite_flow<Flow>) {
            return count_;
        } else {
            return min(flow_.size(), count_);
        }
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> take_adaptor<subflow_t<F>>
    {
        return {flow_.subflow(), count_};
    }

private:
    Flow flow_;
    dist_t count_;
};

}

inline constexpr auto take = [](auto&& flowable, dist_t count)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::take() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).take(count);
};

template <typename D>
constexpr auto flow_base<D>::take(dist_t count) &&
{
    assert(count >= 0 && "Cannot take a negative number of items!");
    return detail::take_adaptor<D>(consume(), count);
}

}

#endif

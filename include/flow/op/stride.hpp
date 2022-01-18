
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_STRIDE_HPP_INCLUDED
#define FLOW_OP_STRIDE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow>
struct stride_adaptor : flow_base<stride_adaptor<Flow>> {

    static constexpr bool is_infinite = is_infinite_flow<Flow>;

    constexpr stride_adaptor(Flow&& flow, dist_t step)
        : flow_(std::move(flow)),
          step_(step)
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (first_) {
            first_ = false;
            return flow_.next();
        }
        return flow_.advance(step_);
    }

    constexpr auto advance(dist_t count) -> next_t<Flow>
    {
        if (first_) {
            first_ = false;
            return flow_.advance(count);
        }
        return flow_.advance(count * step_);
    }

    template <typename F = Flow>
    constexpr auto size() const -> std::enable_if_t<is_sized_flow<F>, dist_t>
    {
        const auto sz = flow_.size();

        return sz/step_ + (sz % step_ != 0);
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> stride_adaptor<subflow_t<F>>
    {
        auto s = stride_adaptor<subflow_t<Flow>>(flow_.subflow(), step_);
        s.first_ = first_;
        return s;
    }

private:
    template <typename>
    friend struct stride_adaptor;

    Flow flow_;
    dist_t step_;
    bool first_ = true;
};

}

inline constexpr auto stride = [](auto&& flowable, dist_t step)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::stride() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).stride(step);
};

template <typename D>
constexpr auto flow_base<D>::stride(dist_t step) &&
{
    assert(step > 0 && "Stride must be positive");
    return detail::stride_adaptor<D>(consume(), step);
}

}

#endif

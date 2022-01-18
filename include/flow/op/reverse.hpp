
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_REVERSE_HPP_INCLUDED
#define FLOW_OP_REVERSE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow>
struct reverse_adaptor : flow_base<reverse_adaptor<Flow>> {

    constexpr explicit reverse_adaptor(Flow&& flow)
        : flow_(std::move(flow))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        return flow_.next_back();
    }

    constexpr auto next_back() -> next_t<Flow>
    {
        return flow_.next();
    }

    template <bool B = is_sized_flow<Flow>>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return flow_.size();
    }

    template <typename F = Flow,
              typename = std::enable_if_t<is_multipass_flow<F>>>
    constexpr auto subflow() const -> reverse_adaptor<subflow_t<F>>
    {
        return reverse_adaptor<Flow>{flow_.subflow()};
    }

    constexpr auto reverse() && -> Flow
    {
        return std::move(*this).flow_;
    }

private:
    Flow flow_;
};

}

inline constexpr auto reverse = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flows::reverse() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).reverse();
};

template <typename D>
constexpr auto flow_base<D>::reverse() &&
{
    static_assert(is_reversible_flow<D>,
                  "reverse() requires a reversible flow");
    return detail::reverse_adaptor<D>{consume()};
}

}

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_COLLECT_HPP_INCLUDED
#define FLOW_OP_COLLECT_HPP_INCLUDED

#include <flow/core/flow_base.hpp>
#include <flow/op/to_range.hpp>

namespace flow {

namespace detail {

template <typename Flow>
struct collector {

    constexpr explicit collector(Flow&& flow)
        : flow_(std::move(flow))
    {}

    template <typename C>
    constexpr operator C() &&
    {
        if constexpr (std::is_constructible_v<C, Flow&&>) {
            return C(std::move(flow_));
        } else {
            using iter_t = decltype(std::move(flow_).to_range().begin());
            static_assert(std::is_constructible_v<C, iter_t, iter_t>,
                          "Incompatible type on LHS of collect()");
            auto rng = std::move(flow_).to_range();
            return C(rng.begin(), rng.end());
        }
    }

private:
    Flow flow_;
};

}

template <typename Flowable>
constexpr auto collect(Flowable&& flowable)
{
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).collect();
}

template <typename Derived>
constexpr auto flow_base<Derived>::collect() &&
{
    return detail::collector<Derived>(consume());
}

}

#endif

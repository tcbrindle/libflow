
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_CYCLE_HPP_INCLUDED
#define FLOW_OP_CYCLE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow>
struct cycle_adaptor : flow_base<cycle_adaptor<Flow>>
{
    static constexpr bool is_infinite = true;

    constexpr explicit cycle_adaptor(Flow&& flow)
        : flow_(std::move(flow))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        while (true) {
            if (auto m = flow_.next()) {
                return m;
            }
            flow_ = saved_;
        }
    }

private:
    Flow flow_;
    Flow saved_ = flow_;
};

}

inline constexpr auto cycle = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>);
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).cycle();
};

template <typename D>
constexpr auto flow_base<D>::cycle() &&
{
    static_assert(std::is_copy_constructible_v<D> && std::is_copy_assignable_v<D>,
                  "cycle() can only be used with flows which are copy-constructible "
                  "and copy-assignable");
    static_assert(!is_infinite_flow<D>,
                  "cycle() cannot be used with infinite flows");
    return detail::cycle_adaptor<D>(consume());
}

}

#endif

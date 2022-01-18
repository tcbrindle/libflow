
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_TAKE_WHILE_HPP_INCLUDED
#define FLOW_OP_TAKE_WHILE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow, typename Pred>
struct take_while_adaptor : flow_base<take_while_adaptor<Flow, Pred>> {

    constexpr take_while_adaptor(Flow&& flow, Pred&& pred)
        : flow_(std::move(flow)),
          pred_(std::move(pred))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (!done_) {
            auto m = flow_.next();
            if (!m.has_value() || !invoke(pred_, std::as_const(*m))) {
                done_ = true;
                return {};
            }
            return m;
        }
        return {};
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> take_while_adaptor<subflow_t<F>, function_ref<Pred>>
    {
        auto s = take_while_adaptor<subflow_t<F>, function_ref<Pred>>{flow_.subflow(), pred_};
        s.done_ = done_;
        return s;
    }

private:
    template <typename, typename>
    friend struct take_while_adaptor;

    Flow flow_;
    Pred pred_;
    bool done_ = false;
};

}

inline constexpr auto take_while = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::take_while() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).take_while(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::take_while(Pred pred) &&
{
    static_assert(std::is_invocable_r_v<bool, Pred&, value_t<D> const&>,
                  "Incompatible predicate passed to take_while()");
    return detail::take_while_adaptor<D, Pred>(consume(), std::move(pred));
}

}

#endif

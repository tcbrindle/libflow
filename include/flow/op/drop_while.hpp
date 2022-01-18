
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_DROP_WHILE_HPP_INCLUDED
#define FLOW_OP_DROP_WHILE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow, typename Pred>
struct drop_while_adaptor : flow_base<drop_while_adaptor<Flow, Pred>> {

    constexpr drop_while_adaptor(Flow&& flow, Pred&& pred)
        : flow_(std::move(flow)),
          pred_(std::move(pred))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (!done_) {
            while (auto m = flow_.next()) {
                if (invoke(pred_, std::as_const(*m))) {
                    continue;
                }
                done_ = true;
                return m;
            }
            return {};
        }
        return flow_.next();
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> drop_while_adaptor<subflow_t<F>, function_ref<Pred>>
    {
        auto d = drop_while_adaptor<subflow_t<Flow>, function_ref<Pred>>{flow_.subflow(), pred_};
        d.done_ = this->done_;
        return d;
    }

private:
    template <typename, typename>
    friend struct drop_while_adaptor;

    Flow flow_;
    Pred pred_;
    bool done_ = false;
};

}

inline constexpr auto drop_while = [](auto&& flowable, auto pred)
{
    static_assert(flow::is_flowable<decltype(flowable)>,
                  "Argument to flow::drop_while() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).drop_while(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::drop_while(Pred pred) &&
{
    static_assert(std::is_invocable_r_v<dist_t, Pred&, value_t<D> const&>,
                  "Incompatible predicate passed to drop_while()");
    return detail::drop_while_adaptor<D, Pred>(consume(), std::move(pred));
}

}

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_FILTER_HPP_INCLUDED
#define FLOW_OP_FILTER_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow, typename Pred>
struct filter_adaptor : flow_base<filter_adaptor<Flow, Pred>> {

    constexpr filter_adaptor(Flow&& flow, Pred&& pred)
        : flow_(std::move(flow)),
          pred_(std::move(pred))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        while (auto m = flow_.next()) {
            if (invoke(pred_, std::as_const(*m))) {
                return m;
            }
        }
        return {};
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> filter_adaptor<subflow_t<F>, function_ref<Pred>>
    {
        return {flow_.subflow(), pred_};
    }

private:
    Flow flow_;
    Pred pred_;
};

} // namespace detail

constexpr auto filter = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::filter() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).filter(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::filter(Pred pred) &&
{
    static_assert(std::is_invocable_r_v<bool, Pred&, value_t<D> const&>,
                  "Incompatible predicate passed to filter()");

    return detail::filter_adaptor<D, Pred>(consume(), std::move(pred));
}

}

#endif

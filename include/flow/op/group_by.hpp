
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_GROUP_BY_HPP_INCLUDED
#define FLOW_OP_GROUP_BY_HPP_INCLUDED

#include <flow/core/flow_base.hpp>
#include <flow/op/take.hpp>

namespace flow {

namespace detail {

template <typename Flow, typename KeyFn>
struct group_by_adaptor : flow_base<group_by_adaptor<Flow, KeyFn>> {
private:
    Flow flow_;
    FLOW_NO_UNIQUE_ADDRESS KeyFn key_fn_;



    using group = take_adaptor<subflow_t<Flow>>;

public:
    constexpr group_by_adaptor(Flow&& flow, KeyFn&& key_fn)
        : flow_(std::move(flow)),
          key_fn_(std::move(key_fn))
    {}

    constexpr auto next() -> maybe<group>
    {
        auto image = flow_.subflow();

        auto m = flow_.next();
        if (!m) {
            return {};
        }
        auto&& last_key = invoke(key_fn_, *m);

        auto peek = flow_.subflow();
        dist_t counter = 1;

        while (true) {
            auto n = peek.next();
            if (!n) {
                break;
            }

            if (invoke(key_fn_, *n) != last_key) {
                break;
            }

            ++counter;
            (void) flow_.next();
        }

        return {std::move(image).take(counter)};
    }

    constexpr auto subflow() & -> group_by_adaptor<subflow_t<Flow>, function_ref<KeyFn>>
    {
        return {flow_.subflow(), function_ref{key_fn_}};
    }
};

}

inline constexpr auto group_by = [](auto&& flowable, auto key_fn)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::group_by() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).group_by(std::move(key_fn));
};

template <typename D>
template <typename Key>
constexpr auto flow_base<D>::group_by(Key key) &&
{
    static_assert(is_multipass_flow<D>,
                  "group_by() requires a multipass flow");
    static_assert(std::is_invocable_v<Key&, item_t<D>>,
                  "Incompatible key function passed to group_by()");
    using R = std::invoke_result_t<Key&, item_t<D>>;
    static_assert(std::is_invocable_v<equal_to, R, R>,
                  "The result type of the key function passed to group_by() "
                  "must be equality comparable");

    return detail::group_by_adaptor<D, Key>(consume(), std::move(key));
}

}

#endif

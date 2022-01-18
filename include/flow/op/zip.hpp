
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_ZIP_HPP_INCLUDED
#define FLOW_OP_ZIP_HPP_INCLUDED

#include <flow/core/flow_base.hpp>
#include <flow/source/iota.hpp>

namespace flow {

namespace detail {

template <typename... Flows>
struct zip_item_type {
    using type = std::tuple<item_t<Flows>...>;
};

template <typename F1, typename F2>
struct zip_item_type<F1, F2> {
    using type = std::pair<item_t<F1>, item_t<F2>>;
};

template <typename... Flows>
using zip_item_t = typename zip_item_type<Flows...>::type;

}

inline constexpr auto zip = [](auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)>,
                  "All arguments to zip() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable0))).zip(FLOW_FWD(flowables)...);
};

inline constexpr auto enumerate = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::enumerate() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).enumerate();
};

template <typename D>
template <typename... Flowables>
constexpr auto flow_base<D>::zip(Flowables&&... flowables) &&
{
    static_assert((is_flowable<Flowables> && ...),
                   "All arguments to zip() must be Flowable");

    return consume().zip_with([](auto&&... items) {
        return detail::zip_item_t<D, flow_t<Flowables>...>(FLOW_FWD(items)...);
    }, FLOW_FWD(flowables)...);
}

template <typename D>
constexpr auto flow_base<D>::enumerate() &&
{
    return flow::ints().zip(consume());
}

}

#endif

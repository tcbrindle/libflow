
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_CARTESIAN_PRODUCT_HPP_INCLUDED
#define FLOW_OP_CARTESIAN_PRODUCT_HPP_INCLUDED

#include <flow/op/zip.hpp>

namespace flow {

inline constexpr auto cartesian_product = [](auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)>,
                  "All arguments to cartesian_product() must be flowable");

    return FLOW_COPY(flow::from(FLOW_FWD(flowable0))).cartesian_product(FLOW_FWD(flowables)...);
};

template <typename D>
template <typename... Fs>
constexpr auto flow_base<D>::cartesian_product(Fs&&... flowables) &&
{
    static_assert((is_flowable<Fs> && ...),
                  "All arguments to cartesian_product() must be flowable");
    static_assert((is_multipass_flow<flow_t<Fs>> && ...),
                  "All flows passed to cartesian_product() must be multipass");

    return consume().cartesian_product_with(
            [](auto&&... items) { return detail::zip_item_t<D, flow_t<Fs>...>(FLOW_FWD(items)...); },
            FLOW_FWD(flowables)...);
}

}

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_OUTPUT_TO_HPP_INCLUDED
#define FLOW_OP_OUTPUT_TO_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr auto output_to = [](auto&& flowable, auto oiter) {
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::output_to() must be Flowable");
    // C++20: static_assert(std::output_iterator<decltype(oiter)>);
    return flow::from(FLOW_FWD(flowable)).output_to(std::move(oiter));
};


template <typename D>
template <typename Iter>
constexpr auto flow_base<D>::output_to(Iter oiter) -> Iter
{
    consume().for_each([&oiter] (auto&& val) {
        *oiter = FLOW_FWD(val);
        ++oiter;
    });
    return oiter;
}

}

#endif

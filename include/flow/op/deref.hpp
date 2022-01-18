
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_DEREF_HPP_INCLUDED
#define FLOW_OP_DEREF_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr auto deref = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::deref must be a flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).deref();
};

template <typename D>
constexpr auto flow_base<D>::deref() &&
{
    const auto bool_check = [](const auto& i) -> decltype(static_cast<bool>(i)) {
        return static_cast<bool>(i);
    };

    static_assert(std::is_invocable_v<decltype(bool_check), item_t<D>>,
                  "Flow's item type is not explicitly convertible to bool");

    return consume().filter(bool_check).unchecked_deref();
}

}

#endif
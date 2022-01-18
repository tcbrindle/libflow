
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_INSPECT_HPP_INCLUDED
#define FLOW_OP_INSPECT_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::inspect(Func func) &&
{
    static_assert(std::is_invocable_v<Func&, value_t<Derived> const&>,
        "Incompatible callable used with inspect()");

    return consume().map([func = std::move(func)] (auto&& val) mutable -> item_t<Derived> {
          (void) invoke(func, std::as_const(val));
          return FLOW_FWD(val);
    });
}

}

#endif

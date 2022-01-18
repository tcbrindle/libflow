
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_FOR_EACH_HPP_INCLUDED

#include <flow/op/fold.hpp>

namespace flow {

inline constexpr auto for_each = [](auto flowable, auto func) {
    static_assert(is_flowable<decltype(flowable)>,
        "First argument to flow::for_each() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).for_each(std::move(func));
};

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::for_each(Func func) -> Func
{
    static_assert(std::is_invocable_v<Func&, item_t<Derived>>,
                  "Incompatible callable passed to for_each()");
    consume().fold([&func](bool, auto&& val) {
      (void) invoke(func, FLOW_FWD(val));
      return true;
    }, true);
    return func;
}

}

#endif


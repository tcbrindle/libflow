
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_TRY_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_TRY_FOR_EACH_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr auto try_for_each = [](auto flowable, auto func) {
    static_assert(is_flowable<decltype(flowable)>);
    return flow::from(FLOW_FWD(flowable), std::move(func));
};

template <typename D>
template <typename Func>
constexpr auto flow_base<D>::try_for_each(Func func)
{
    using result_t = std::invoke_result_t<Func&, next_t<D>&&>;
    return derived().try_fold([&func](auto&& /*unused*/, auto&& m) -> decltype(auto) {
      return invoke(func, FLOW_FWD(m));
    }, result_t{});
}

}

#endif

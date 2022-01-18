
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_TRY_FOLD_HPP_INCLUDED
#define FLOW_OP_TRY_FOLD_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr auto try_fold = [](auto&& flowable, auto func, auto init) {
    static_assert(is_flowable<decltype(flowable)>);
    return flow::from(FLOW_FWD(flowable)).try_fold(std::move(func), std::move(init));
};

template <typename D>
template <typename Func, typename Init>
constexpr auto flow_base<D>::try_fold(Func func, Init init) -> Init
{
    static_assert(std::is_invocable_r_v<Init, Func&, Init&&, next_t<D>&&>,
                  "Incompatible callable passed to try_fold()");
    while (auto m = derived().next()) {
        init = invoke(func, std::move(init), std::move(m));
        if (!static_cast<bool>(init)) {
            break;
        }
    }
    return init;
}

}

#endif

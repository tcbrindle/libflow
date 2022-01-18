
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_EQUAL_HPP_INCLUDED
#define FLOW_OP_EQUAL_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct equal_fn {
    template <typename F1, typename F2, typename Cmp = equal_to>
    constexpr auto operator()(F1&& f1, F2&& f2, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<F1> && is_flowable<F2>,
                      "Arguments to flow::equal() must be Flowable types");
        return flow::from(FLOW_FWD(f1)).equal(FLOW_FWD(f2), std::move(cmp));
    }

};


}

inline constexpr auto equal = detail::equal_fn{};

template <typename D>
template <typename Flowable, typename Cmp>
constexpr auto flow_base<D>::equal(Flowable&& flowable, Cmp cmp) -> bool
{
    static_assert(is_flowable<Flowable>,
                  "Argument to equal() must be a Flowable type");
    static_assert(std::is_invocable_r_v<bool, Cmp&, item_t<D>, flow_item_t<Flowable>>,
                  "Incompatible comparator passed to equal()");

    auto&& other = flow::from(FLOW_FWD(flowable));

    while (true) {
        auto m1 = derived().next();
        auto m2 = other.next();

        if (!m1) {
            return !static_cast<bool>(m2);
        }
        if (!m2) {
            return false;
        }

        if (!cmp(*m1, *m2)) {
            return false;
        }
    }
}

}

#endif

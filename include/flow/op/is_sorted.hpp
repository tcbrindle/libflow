
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_IS_SORTED_HPP_INCLUDED
#define FLOW_OP_IS_SORTED_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct is_sorted_fn {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::is_sorted() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).is_sorted(std::move(cmp));
    }
};

}

inline constexpr auto is_sorted = detail::is_sorted_fn{};

template <typename D>
template <typename Cmp>
constexpr bool flow_base<D>::is_sorted(Cmp cmp)
{
    auto last = derived().next();
    if (!last) {
        // An empty flow is sorted
        return true;
    }

    return derived().try_fold([&cmp, &last] (bool, next_t<D>&& next) {
        if (invoke(cmp, *next, *last)) {
            return false;
        } else {
            last = std::move(next);
            return true;
        }
    }, true);
}

}

#endif

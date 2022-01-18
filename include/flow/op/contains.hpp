
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_CONTAINS_HPP_INCLUDED
#define FLOW_OP_CONTAINS_HPP_INCLUDED

#include <flow/op/find.hpp>

namespace flow {

namespace detail {

struct contains_op {
    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& item, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::contains() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).contains(item, std::move(cmp));
    }
};

}

inline constexpr auto contains = detail::contains_op{};

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::contains(const T &item, Cmp cmp) -> bool
{
    static_assert(std::is_invocable_v<Cmp&, value_t<Derived> const&, const T&>,
        "Incompatible comparator used with contains()");
    static_assert(std::is_invocable_r_v<bool, Cmp&, value_t<Derived> const&, const T&>,
        "Comparator used with contains() must return bool");

    return derived().any([&item, &cmp] (auto const& val) {
          return invoke(cmp, val, item);
    });
}

}

#endif

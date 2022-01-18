
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_SCAN_HPP_INCLUDED
#define FLOW_OP_SCAN_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Base, typename Func, typename Init>
struct scan_adaptor : flow_base<scan_adaptor<Base, Func, Init>> {

    constexpr scan_adaptor(Base base, Func func, Init init)
        : base_(std::move(base)),
          func_(std::move(func)),
          state_(std::move(init))
    {}

    constexpr auto next() -> maybe<Init>
    {
        return base_.next().map([&](auto&& e) {
            state_ = invoke(func_, state_, FLOW_FWD(e));
            return state_;
        });
    }

private:
    Base base_;
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    Init state_;
};

}

template <typename Derived>
template <typename Func, typename, typename Init>
constexpr auto flow_base<Derived>::scan(Func func, Init init) &&
{
    return detail::scan_adaptor{consume(), std::move(func), std::move(init)};
}

template <typename Derived>
constexpr auto flow_base<Derived>::partial_sum() &&
{
    return consume().scan(std::plus<>{});
}

}

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_FOLD_HPP_INCLUDED
#define FLOW_OP_FOLD_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct fold_op {

    template <typename Flowable, typename Func, typename Init>
    constexpr auto operator()(Flowable&& flowable, Func func, Init init) const
    {
        static_assert(is_flowable<Flowable>,
                "First argument to flow::fold() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold(std::move(func), std::move(init));
    }

    template <typename Flowable, typename Func>
    constexpr auto operator()(Flowable&& flowable, Func func) const
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::fold() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold(std::move(func));
    }
};

struct fold_first_op {
    template <typename Flowable, typename Func>
    constexpr auto operator()(Flowable&& flowable, Func func) const
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::fold_first must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold_first(std::move(func));
    }

};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};

inline constexpr auto fold_first = detail::fold_first_op{};

template <typename Derived>
template <typename Func, typename Init>
constexpr auto flow_base<Derived>::fold(Func func, Init init) -> Init
{
    static_assert(std::is_invocable_v<Func&, Init&&, item_t<Derived>>,
                  "Incompatible callable passed to fold()");
    static_assert(std::is_assignable_v<Init&, std::invoke_result_t<Func&, Init&&, item_t<Derived>>>,
                  "Accumulator of fold() is not assignable from the result of the function");
    static_assert(!is_infinite_flow<Derived>,
                  "Cannot perform a fold over an infinite flow");

    struct always {
        Init val;
        constexpr explicit operator bool() const { return true; }
    };

    return derived().try_fold([&func](always acc, auto m) {
        return always{func(std::move(acc).val, *std::move(m))};
    }, always{std::move(init)}).val;
}

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::fold(Func func)
{
    static_assert(std::is_default_constructible_v<value_t<Derived>>,
            "This flow's value type is not default constructible. "
            "Use the fold(function, initial_value) overload instead");
    return consume().fold(std::move(func), value_t<Derived>{});
}

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::fold_first(Func func)
{
    using return_t = maybe<value_t<Derived>>;

    auto val = derived().next();
    if (!val) {
        return return_t{};
    }

    return return_t{derived().fold(std::move(func), *std::move(val))};
}

}

#endif


#ifndef FLOW_OP_FOLD_HPP_INCLUDED
#define FLOW_OP_FOLD_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct fold_op {

    template <typename Flow, typename Func, typename Init>
    constexpr auto operator()(Flow flow, Func func, Init init) const
    {
        static_assert(std::is_invocable_v<Func&, item_t<Flow>, Init&&>,
                      "Incompatible callable passed to fold()");
        static_assert(std::is_assignable_v<Init&, std::invoke_result_t<Func&, item_t<Flow>, Init&&>>,
                      "Accumulator of fold() is not assignable from the result of the function");

        struct always {
            Init val;
            constexpr explicit operator bool() const { return true; }
        };

        return std::move(flow).try_fold([&func](always acc, auto m) {
          return always{func(std::move(acc).val, *std::move(m))};
        }, always{std::move(init)}).val;
    }

    template <typename Flow, typename Func>
    constexpr auto operator()(Flow flow, Func func) const
    {
        return fold_op{}(std::move(flow), std::move(func), value_t<Flow>{});
    }
};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};

template <typename Derived>
template <typename Func, typename Init>
constexpr auto flow_base<Derived>::fold(Func func, Init init) && -> Init
{
    return flow::fold(consume(), std::move(func), std::move(init));
}

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::fold(Func func) &&
{
    return flow::fold(consume(), std::move(func));
}

}

#endif


#ifndef FLOW_OP_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_FOR_EACH_HPP_INCLUDED

#include <flow/op/fold.hpp>

namespace flow {

namespace detail {

struct for_each_op {

    template <typename Flow, typename Func>
    constexpr auto operator()(Flow flow, Func func) const
    {
        static_assert(std::is_invocable_v<Func&, item_t<Flow>>,
            "Incompatible callable passed to for_each()");
        fold(std::move(flow), [&func](bool, auto&& val) {
            func(FLOW_FWD(val));
            return true;
        }, true);
        return func;
    }

};

}

inline constexpr auto for_each = detail::for_each_op{};

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::for_each(Func func) &&
{
    return flow::for_each(consume(), std::move(func));
}

}

#endif


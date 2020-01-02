
#ifndef FLOW_OP_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_FOR_EACH_HPP_INCLUDED

#include <flow/op/fold.hpp>

namespace flow {

namespace detail {

struct for_each_op {

    template <typename Flow, typename Func>
    constexpr auto operator()(Flow flow, Func func) const
    {
        static_assert(is_flow<Flow>,
            "First argument to flow::for_each() must be a Flow");
        return std::move(flow).for_each(std::move(func));
    }
};

}

inline constexpr auto for_each = detail::for_each_op{};

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::for_each(Func func) &&
{
    static_assert(std::is_invocable_v<Func&, item_t<Derived>>,
                  "Incompatible callable passed to for_each()");
    consume().fold([&func](bool, auto&& val) {
      (void) func(FLOW_FWD(val));
      return true;
    }, true);
    return func;
}

}

#endif


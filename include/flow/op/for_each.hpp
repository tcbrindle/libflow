
#ifndef FLOW_OP_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_FOR_EACH_HPP_INCLUDED

#include <flow/op/fold.hpp>

namespace flow {

inline constexpr auto for_each = [](auto flow, auto func) {
    static_assert(is_flow<decltype(flow)>,
        "First argument to flow::for_each() must be a Flow");
    return std::move(flow).for_each(std::move(func));
};

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



#ifndef FLOW_OP_COUNT_IF_HPP_INCLUDED
#define FLOW_OP_COUNT_IF_HPP_INCLUDED

#include <flow/op/fold.hpp>

namespace flow {

inline constexpr auto count_if = [](auto flow, auto pred)
{
    static_assert(is_flow<decltype(flow)>,
        "First argument to flow::count_if must be a Flow");
    return std::move(flow).count_if(std::move(pred));
};

template <typename Derived>
template <typename Pred>
constexpr auto flow_base<Derived>::count_if(Pred pred) && -> dist_t
{
    static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                  "Predicate must be callable with the Flow's item_type,"
                  " and must return bool");
    return consume().fold([&pred](dist_t count, auto&& val) {
      return count + static_cast<dist_t>(pred(FLOW_FWD(val)));
    }, dist_t{0});
}

} // namespace flow

#endif

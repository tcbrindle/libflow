
#ifndef FLOW_OP_MAP_HPP_INCLUDED
#define FLOW_OP_MAP_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow, typename Func>
struct map_adaptor : flow_base<map_adaptor<Flow, Func>> {

    using item_type = std::invoke_result_t<Func&, item_t<Flow>>;

    constexpr map_adaptor(Flow&& flow, Func func)
        : flow_(std::move(flow)),
          func_(std::move(func))
    {}

    constexpr auto next() -> maybe<item_type>
    {
        return flow_.next().map(func_);
    }

    constexpr auto advance(dist_t dist) -> maybe<item_type>
    {
        return flow_.advance(dist).map(func_);
    }

private:
    Flow flow_;
    FLOW_NO_UNIQUE_ADDRESS Func func_;
};

}

inline constexpr auto map = [](auto&& flowable, auto&& func)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::map must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).map(FLOW_FWD(func));
};

template <typename D>
template <typename Func>
constexpr auto flow_base<D>::map(Func func) &&
{
    static_assert(std::is_invocable_v<Func&, item_t<D>>,
        "Incompatible callable passed to map()");
    static_assert(!std::is_void_v<std::invoke_result_t<Func&, item_t<D>>>,
        "Map cannot be used with a function returning void");

    return detail::map_adaptor<D, Func>(consume(), std::move(func));
}

}

#endif

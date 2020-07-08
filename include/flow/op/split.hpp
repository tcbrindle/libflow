
#ifndef FLOW_OP_SPLIT_HPP_INCLUDED
#define FLOW_OP_SPLIT_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr auto split = [](auto&& flowable, flow_value_t<decltype(flowable)> delim)
{
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).split(delim);
};

template <typename Derived>
template <typename D>
constexpr auto flow_base<Derived>::split(value_t<D> delim) &&
{
    return consume().group_by(flow::pred::eq(std::move(delim))).stride(2);
}

}

#endif

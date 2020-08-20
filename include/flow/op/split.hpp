
#ifndef FLOW_OP_SPLIT_HPP_INCLUDED
#define FLOW_OP_SPLIT_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct split_op {
    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable, flow_value_t<Flowable> delim) const
    {
        return FLOW_COPY(flow::from(FLOW_FWD(flowable))).split(delim);
    }
};

}

inline constexpr auto split = detail::split_op{};

template <typename Derived>
template <typename D>
constexpr auto flow_base<Derived>::split(value_t<D> delim) &&
{
    return consume().group_by(flow::pred::eq(std::move(delim))).stride(2);
}

}

#endif

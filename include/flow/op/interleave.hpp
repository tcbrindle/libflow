
#ifndef FLOW_OP_INTERLEAVE_HPP_INCLUDED
#define FLOW_OP_INTERLEAVE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Flow1, typename Flow2>
struct interleave_adaptor : flow_base<interleave_adaptor<Flow1, Flow2>> {

    constexpr interleave_adaptor(Flow1&& flow1, Flow2&& flow2)
        : flow1_(std::move(flow1)),
          flow2_(std::move(flow2))
    {}

    constexpr auto next() -> next_t<Flow1>
    {
        auto item = first_ ? flow1_.next() : flow2_.next();
        first_ = !first_;
        return item;
    }

    template <typename F1 = Flow1, typename F2 = Flow2>
    constexpr auto subflow() & -> interleave_adaptor<subflow_t<F1>, subflow_t<F2>>
    {
        auto i = interleave_adaptor<subflow_t<F1>, subflow_t<F2>>(flow1_.subflow(), flow2_.subflow());
        i.first_ = first_;
        return i;
    }

private:
    template <typename, typename>
    friend struct interleave_adaptor;

    Flow1 flow1_;
    Flow2 flow2_;
    bool first_ = true;
};

}

template <typename Derived>
template <typename Flow>
constexpr auto flow_base<Derived>::interleave(Flow with) &&
{
    static_assert(is_flow<Flow>, "Argument to interleave() must be a Flow!");
    static_assert(std::is_same_v<item_t<Derived>, item_t<Flow>>,
        "Flows used with interleave() must have the exact same item type");
    return detail::interleave_adaptor<Derived, Flow>(consume(), std::move(with));
}

}

#endif

#ifndef FLOW_OP_COLLECT_HPP_INCLUDED
#define FLOW_OP_COLLECT_HPP_INCLUDED

#include <flow/core/flow_base.hpp>
#include <flow/op/to_range.hpp>

namespace flow {

namespace detail {

template <typename Flow>
struct collector {

    constexpr collector(Flow&& flow)
        : flow_(std::move(flow))
    {}

    template <typename C>
    constexpr operator C() &&
    {
        using iter_t = decltype(std::move(flow_).to_range().begin());
        static_assert(std::is_constructible_v<C, iter_t, iter_t>,
            "Incompatible type on LHS of collect()");
        auto rng = std::move(flow_).to_range();
        return C(rng.begin(), rng.end());
    }

private:
    Flow flow_;
};

}

template <typename Derived>
template <typename>
constexpr auto flow_base<Derived>::collect() &&
{
    return detail::collector<Derived>(consume());
}

}

#endif


#ifndef FLOW_OP_CONTAINS_HPP_INCLUDED
#define FLOW_OP_CONTAINS_HPP_INCLUDED

#include <flow/op/find.hpp>

namespace flow {

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::contains(const T &item, Cmp cmp) -> bool
{
    static_assert(std::is_invocable_v<Cmp&, value_t<Derived> const&, const T&>,
        "Incompatible comparator used with contains()");
    static_assert(std::is_invocable_r_v<bool, Cmp&, value_t<Derived> const&, const T&>,
        "Comparator used with contains() must return bool");

    return derived().any([&item, &cmp] (auto const& val) {
          return invoke(cmp, val, item);
    });
}

namespace detail {

struct contains_op {
    template <typename Flow, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flow&& flow, const T& item, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flow<remove_cvref_t<Flow>>,
            "First argument to flow::contains() must be a Flow");
        return FLOW_FWD(flow).contains(item, std::move(cmp));
    }
};

}

inline constexpr auto contains = detail::contains_op{};

}

#endif

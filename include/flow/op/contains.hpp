
#ifndef FLOW_OP_CONTAINS_HPP_INCLUDED
#define FLOW_OP_CONTAINS_HPP_INCLUDED

#include <flow/op/find.hpp>

namespace flow {

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::contains(const T &item, Cmp cmp) -> bool
{
    static_assert(std::is_invocable_v<Cmp&, item_t<Derived>&, const T&>,
        "Incompatible comparator used with contains()");
    static_assert(std::is_invocable_r_v<bool, Cmp&, item_t<Derived>&, const T&>,
        "Comparator used with contains() must return bool");

    return !derived().try_fold([&item, &cmp](bool, auto m) -> bool {
        return !invoke(cmp, *m, item);
    }, true);
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

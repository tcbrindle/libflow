
#ifndef FLOW_OP_COUNT_HPP_INCLUDED
#define FLOW_OP_COUNT_HPP_INCLUDED

#include <flow/op/count_if.hpp>

namespace flow {

namespace detail {

struct count_op {

    template <typename Flow>
    constexpr auto operator()(Flow flow) const
    {
        static_assert(is_flow<Flow>,
            "Argument to flow::count() must be a Flow");
        return std::move(flow).count();
    }

    template <typename Flow, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flow flow, const T& item, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flow<Flow>,
                      "Argument to flow::count() must be a Flow");
        return std::move(flow).count(item, std::move(cmp));
    }

};

}

inline constexpr auto count = detail::count_op{};

template <typename Derived>
constexpr auto flow_base<Derived>::count() && -> dist_t
{
    return consume().count_if([](auto const& /*unused*/) {
        return true;
    });
}

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::count(const T& item, Cmp cmp) && -> dist_t
{
    using const_item_t = std::add_lvalue_reference_t<
        std::add_const_t<std::remove_reference_t<item_t<Derived>>>>;
    static_assert(std::is_invocable_r_v<bool, Cmp&, const T&, const_item_t>,
        "Incompatible comparator used with count()");

    return consume().count_if([&item, &cmp] (auto const& val) {
        return invoke(cmp, item, val);
    });
};

}

#endif

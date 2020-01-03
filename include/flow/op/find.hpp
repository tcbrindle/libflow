
#ifndef FLOW_OP_FIND_HPP_INCLUDED
#define FLOW_OP_FIND_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::find(const T& item, Cmp cmp)
{
    struct out {
        maybe<item_t<Derived>> val{};
        constexpr explicit operator bool() const { return !val; }
    };

    return derived().try_for_each([&item, &cmp](auto m) {
      return invoke(cmp, *m, item) ? out{std::move(m)} : out{};
    }).val;
}

namespace detail {

struct find_op {

    template <typename Flow, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flow&& flow, const T& item, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flow<remove_cvref_t<decltype(flow)>>,
                      "First argument to flow::find() must be a Flow");
        return FLOW_FWD(flow).find(item, std::move(cmp));
    }

};

}

inline constexpr auto find = detail::find_op{};

}

#endif

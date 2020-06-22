
#ifndef FLOW_OP_FIND_HPP_INCLUDED
#define FLOW_OP_FIND_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct find_op {

    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& item, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::find() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).find(item, std::move(cmp));
    }

};

} // namespace detail

inline constexpr auto find = detail::find_op{};

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::find(const T& item, Cmp cmp)
{
    struct out {
        next_t<Derived> val{};
        constexpr explicit operator bool() const { return !val; }
    };

    return derived().try_for_each([&item, &cmp](auto m) {
      return invoke(cmp, *m, item) ? out{std::move(m)} : out{};
    }).val;
}

}

#endif


#ifndef FLOW_SOURCE_INTS_HPP_INCLUDED
#define FLOW_SOURCE_INTS_HPP_INCLUDED

#include <flow/source/generate.hpp>
#include <flow/core/flow_base.hpp>

namespace flow {

constexpr inline struct {

    template <typename I>
    constexpr auto operator()(I from = I{}) const
    {
        constexpr auto post_inc = [](auto& i) { i++; };
        static_assert(std::is_invocable_v<decltype(post_inc), I&>);
        return generate([val = from] () mutable { return val++; });
    }

    template <typename I>
    constexpr auto operator()(I from, I upto) const
    {
        static_assert(std::is_invocable_v<std::less<>, const I&, const I&>);
        return generate([val = from] () mutable { return val++; })
            .take_while([limit = upto] (const I& val) {
                return  std::less<>{}(val, limit);
            });
    }

} iota;

constexpr inline struct {
    constexpr auto operator()(dist_t from = 0) const
    {
        return iota(from);
    }

    constexpr auto operator()(dist_t from, dist_t upto) const
    {
        return iota(from).take(upto - from);
    }

} ints;

}

#endif

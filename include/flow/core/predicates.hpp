
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_CORE_PREDICATES_HPP_INCLUDED
#define FLOW_CORE_PREDICATES_HPP_INCLUDED

#include <flow/core/functional.hpp>
#include <flow/core/macros.hpp>

#include <type_traits>

namespace flow::pred {

namespace detail {

template <typename Lambda>
struct predicate : Lambda {};

template <typename Lambda>
predicate(Lambda&&) -> predicate<Lambda>;

template <typename Lambda>
constexpr auto make_predicate(Lambda&& lambda)
{
    return predicate<Lambda>{FLOW_FWD(lambda)};
}

// This could/should be a lambda, but it confuses MSVC
template <typename Op>
struct cmp {
    template <typename T>
    constexpr auto operator()(T&& val) const
    {
        return predicate{[val = FLOW_FWD(val)](const auto& other) {
            return Op{}(other, val);
        }};
    }
};

} // namespace detail

/// Given a predicate, returns a new predicate with the condition reversed
inline constexpr auto not_ = [](auto&& pred) {
    return detail::make_predicate([p = FLOW_FWD(pred)] (auto const&... args) {
        return !invoke(p, FLOW_FWD(args)...);
    });
};

/// Returns a new predicate which is satisifed only if both the given predicates
/// return `true`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `false`, the second will not be evaluated.
inline constexpr auto both = [](auto&& p, auto&& and_) {
    return detail::predicate{[p1 = FLOW_FWD(p), p2 = FLOW_FWD(and_)] (auto const&... args) {
        return invoke(p1, args...) && invoke(p2, args...);
    }};
};

/// Returns a new predicate which is satisifed only if either of the given
/// predicates return `true`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `true`, the second will not be evaluated
inline constexpr auto either = [](auto&& p, auto&& or_) {
     return detail::predicate{[p1 = FLOW_FWD(p), p2 = FLOW_FWD(or_)] (auto const&... args) {
        return invoke(p1, args...) || invoke(p2, args...);
     }};
};

namespace detail {

template <typename P>
constexpr auto operator!(detail::predicate<P> pred)
{
    return not_(std::move(pred));
}

template <typename L, typename R>
constexpr auto operator&&(detail::predicate<L> lhs, detail::predicate<R> rhs)
{
    return both(std::move(lhs), std::move(rhs));
}

template <typename L, typename R>
constexpr auto operator||(detail::predicate<L> lhs, detail::predicate<R> rhs)
{
    return either(std::move(lhs), std::move(rhs));
}

}

/// Returns a new predicate with is satified only if both of the given
/// predicates return `false`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `true`, the second will not be evaluated.
inline constexpr auto neither = [](auto&& p1, auto&& nor) {
    return not_(either(FLOW_FWD(p1), FLOW_FWD(nor)));
};

inline constexpr auto eq = detail::cmp<flow::equal_to>{};
inline constexpr auto neq = detail::cmp<flow::not_equal_to>{};
inline constexpr auto lt = detail::cmp<flow::less>{};
inline constexpr auto gt = detail::cmp<flow::greater>{};
inline constexpr auto leq = detail::cmp<flow::less_equal>{};
inline constexpr auto geq = detail::cmp<flow::greater_equal>{};

/// Returns true if the given value is greater than a zero of the same type.
inline constexpr auto positive = detail::make_predicate([](auto const& val) -> bool {
    return val > decltype(val){0};
});

/// Returns true if the given value is less than a zero of the same type.
inline constexpr auto negative = detail::make_predicate([](auto const& val) -> bool {
    return val < decltype(val){0};
});

/// Returns true if the given value is not equal to a zero of the same type.
inline constexpr auto nonzero = detail::make_predicate([](auto const& val) -> bool {
    return val != decltype(val){0};
});

/// Given a sequence of values, constructs a predicate which returns true
/// if its argument compares equal to one of the values
inline constexpr auto in = [](auto const&... vals) {
    static_assert(sizeof...(vals) > 0);
    return detail::predicate{[vals...](auto const& arg) -> decltype(auto) {
        return (equal_to{}(arg, vals) || ...);
    }};
};

inline constexpr auto even = detail::make_predicate([](auto const& val) -> bool {
    return val % decltype(val){2} == decltype(val){0};
});

inline constexpr auto odd = detail::make_predicate([](auto const& val) -> bool {
  return val % decltype(val){2} != decltype(val){0};
});

} // namespaces

#endif

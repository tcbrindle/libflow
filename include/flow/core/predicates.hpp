
#ifndef FLOW_CORE_COMPARATORS_HPP_INCLUDED
#define FLOW_CORE_COMPARATORS_HPP_INCLUDED

#include <flow/core/macros.hpp>
#include <flow/core/invoke.hpp>

#include <functional>
#include <type_traits>

namespace flow::pred {

namespace detail {

template <typename Lambda>
struct predicate : Lambda {};

template <typename Lambda>
predicate(Lambda&&) -> predicate<Lambda>;

template <typename Op>
inline constexpr auto cmp = [](auto&& val) {
    return predicate{[val = FLOW_FWD(val)](auto const& other) {
        return Op{}(other, val);
    }};
};

} // namespace detail

/// Given a predicate, returns a new predicate with the condition reversed
inline constexpr auto not_ = [](auto&& pred) {
    return detail::predicate{[p = FLOW_FWD(pred)] (auto const&... args) {
        return !invoke(p, FLOW_FWD(args)...);
    }};
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

inline constexpr auto eq = detail::cmp<std::equal_to<>>;
inline constexpr auto neq = detail::cmp<std::not_equal_to<>>;
inline constexpr auto lt = detail::cmp<std::less<>>;
inline constexpr auto gt = detail::cmp<std::greater<>>;
inline constexpr auto leq = detail::cmp<std::less_equal<>>;
inline constexpr auto geq = detail::cmp<std::greater_equal<>>;

/// Returns true if the given value is greater than a zero of the same type.
inline constexpr auto positive = detail::predicate{[](auto const& val) -> bool {
    return val > decltype(val){0};
}};

/// Returns true if the given value is less than a zero of the same type.
inline constexpr auto negative = detail::predicate{[](auto const& val) -> bool {
    return val < decltype(val){0};
}};

/// Returns true if the given value is not equal to a zero of the same type.
inline constexpr auto nonzero = detail::predicate{[](auto const& val) -> bool {
    return val != decltype(val){0};
}};

/// Given a sequence of values, constructs a predicate which returns true
/// if its argument compares equal to one of the values
inline constexpr auto in = [](auto const&... vals) {
    static_assert(sizeof...(vals) > 0);
    return detail::predicate{[vals...](auto const& arg) -> decltype(auto) {
        return (std::equal_to<>{}(arg, vals) || ...);
    }};
};

} // namespaces

#endif

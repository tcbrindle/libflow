
#ifndef FLOW_HPP_INCLUDED
#define FLOW_HPP_INCLUDED


#ifndef FLOW_CORE_FLOW_BASE_HPP_INCLUDED
#define FLOW_CORE_FLOW_BASE_HPP_INCLUDED


#ifndef FLOW_CORE_INVOKE_HPP_INCLUDED
#define FLOW_CORE_INVOKE_HPP_INCLUDED


#ifndef FLOW_CORE_MACROS_HPP_INCLUDED
#define FLOW_CORE_MACROS_HPP_INCLUDED

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(no_unique_address)
#define FLOW_HAS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
#endif

#ifdef FLOW_HAS_NO_UNIQUE_ADDRESS
#define FLOW_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define FLOW_NO_UNIQUE_ADDRESS
#endif

#if defined(__cpp_coroutines)
#define FLOW_HAVE_COROUTINES
#endif

#define FLOW_FWD(x) (static_cast<decltype(x)&&>(x))

#define FLOW_FOR(VAR_DECL, ...) \
    if (auto&& _flow = (__VA_ARGS__); true) \
        while (auto _a = _flow.next()) \
            if (VAR_DECL = *std::move(_a); true)

#endif


#include <functional> // for reference_wrapper :-(

namespace flow {

namespace detail {

template <typename>
inline constexpr bool is_reference_wrapper = false;

template <typename R>
inline constexpr bool is_reference_wrapper<std::reference_wrapper<R>> = true;

struct invoke_fn {
private:
    template <typename T, typename Type, typename T1, typename... Args>
    static constexpr auto impl(Type T::* f, T1&& t1, Args&&... args) -> decltype(auto)
    {
        if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
            if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
                return (FLOW_FWD(t1).*f)(FLOW_FWD(args)...);
            } else if constexpr (is_reference_wrapper<std::decay_t<T1>>) {
                return (t1.get().*f)(FLOW_FWD(args)...);
            } else {
                return ((*FLOW_FWD(t1)).*f)(FLOW_FWD(args)...);
            }
        } else {
            static_assert(std::is_member_object_pointer_v<decltype(f)>);
            static_assert(sizeof...(args) == 0);
            if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
                return FLOW_FWD(t1).*f;
            } else if constexpr (is_reference_wrapper<std::decay_t<T1>>) {
                return t1.get().*f;
            } else {
                return (*FLOW_FWD(t1)).*f;
            }
        }
    }

    template <typename F, typename... Args>
    static constexpr auto impl(F&& f, Args&&... args) -> decltype(auto)
    {
        return FLOW_FWD(f)(FLOW_FWD(args)...);
    }

public:
    template <typename F, typename... Args>
    constexpr auto operator()(F&& func, Args&&... args) const
        noexcept(std::is_nothrow_invocable_v<F, Args...>)
        -> std::invoke_result_t<F, Args...>
    {
        return impl(FLOW_FWD(func), FLOW_FWD(args)...);
    }
};

}

inline constexpr auto invoke = detail::invoke_fn{};

}

#endif


#ifndef FLOW_CORE_MAYBE_HPP_INCLUDED
#define FLOW_CORE_MAYBE_HPP_INCLUDED



#include <optional>

namespace flow {

namespace detail {

template <typename T>
struct maybe_move_assign_base : std::optional<T>
{
    using std::optional<T>::optional;

    constexpr maybe_move_assign_base() = default;
    constexpr maybe_move_assign_base(maybe_move_assign_base&&) = default;
    constexpr maybe_move_assign_base(maybe_move_assign_base const&) = default;

    ~maybe_move_assign_base() = default;

    constexpr maybe_move_assign_base& operator=(maybe_move_assign_base&& other)
        noexcept(noexcept(std::is_nothrow_move_constructible_v<T>))
    {
        if (other) {
            as_opt() = std::optional<T>{std::in_place, *std::move(other)};
        } else {
            as_opt() = std::optional<T>{std::nullopt};
        }
        return *this;
    }

protected:
    constexpr auto& as_opt() & { return static_cast<std::optional<T>&>(*this); }
    constexpr auto& as_opt() const& { return static_cast<std::optional<T> const&>(*this); }
};

template <typename T, bool IsCopyable = std::is_copy_constructible_v<T>>
struct maybe_copy_assign_base : maybe_move_assign_base<T> {

    using maybe_move_assign_base<T>::maybe_move_assign_base;

    constexpr maybe_copy_assign_base() = default;
    constexpr maybe_copy_assign_base(maybe_copy_assign_base&&) = default;
//    constexpr maybe_copy_assign_base& operator=(maybe_copy_assign_base&&) = default;
    // Urgh, Clang doesn't think the defaulted definition is constexpr for some reason
    constexpr maybe_copy_assign_base& operator=(maybe_copy_assign_base&& other)
        noexcept(std::is_nothrow_move_assignable_v<maybe_move_assign_base<T>>)
    {
        static_cast<maybe_move_assign_base<T>&>(*this)
                = static_cast<maybe_move_assign_base<T>&&>(other);
        return *this;
    }

    constexpr maybe_copy_assign_base(maybe_copy_assign_base const&) = default;
    ~maybe_copy_assign_base() = default;

    constexpr maybe_copy_assign_base& operator=(maybe_copy_assign_base const& other)
    {
        if (this != &other) {
            if (other.as_opt()) {
                this->as_opt() = std::optional<T>{std::in_place, *other.as_opt()};
            } else {
                this->as_opt() = std::optional<T>{std::nullopt};
            }
        }
        return *this;
    }
};

template <typename T>
struct maybe_copy_assign_base<T, false> : maybe_move_assign_base<T> {
    using maybe_move_assign_base<T>::maybe_move_assign_base;
};

}

template <typename T>
class maybe : detail::maybe_copy_assign_base<T>  {

    using base = detail::maybe_copy_assign_base<T>;

public:
    using value_type = T;

    constexpr maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(const U& value) : base{std::in_place, value} {}

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U&& value) : base{std::in_place, std::move(value)} {}

    constexpr maybe(maybe const&) = default;
    constexpr maybe& operator=(const maybe&) = default;
    constexpr maybe(maybe&&) = default;
    constexpr maybe& operator=(maybe&&) = default;

    using std::optional<T>::operator*;
    using std::optional<T>::operator->;
    using std::optional<T>::value;
    using std::optional<T>::has_value;

#ifndef _MSC_VER
    using std::optional<T>::operator bool;
#else
    constexpr explicit operator bool() const {
        return this->has_value();
    }
#endif

    // Override optional::reset to make it constexpr
    constexpr void reset() { *this = maybe<T>{}; }

    template <typename Func>
    constexpr auto map(Func&& func) & { return maybe::map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const& { return maybe::map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) && { return maybe::map_impl(std::move(*this), FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const&& { return maybe::map_impl(std::move(*this), FLOW_FWD(func)); }

private:
    template <typename Self, typename F>
    static constexpr auto map_impl(Self&& self, F&& func)
        -> maybe<std::invoke_result_t<F, decltype(*FLOW_FWD(self))>>
    {
        if (self) {
            return invoke(FLOW_FWD(func), *FLOW_FWD(self));
        }
        return {};
    }
};

template <typename T>
struct maybe<T&> {
    using value_type = T&;

    constexpr maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U& item) : ptr_(std::addressof(item)) {}

    maybe(T&&) = delete;

    constexpr T& operator*() const { return *ptr_; }
    constexpr T* operator->() const { return ptr_; }

    constexpr T& value() const
    {
        if (!ptr_) {
            throw std::bad_optional_access{};
        }
        return *ptr_;
    }

    constexpr auto reset() { ptr_ = nullptr; }

    constexpr auto has_value() const -> bool { return ptr_ != nullptr; }
    explicit constexpr operator bool() const { return has_value(); }

    template <typename Func>
    constexpr auto map(Func&& func) const -> maybe<std::invoke_result_t<Func, T&>>
    {
        if (ptr_) {
            return {invoke(FLOW_FWD(func), *ptr_)};
        }
        return {};
    }

private:
    T* ptr_ = nullptr;
};

template <typename T>
struct maybe<T&&>
{
    using value_type = T&&;

    constexpr maybe() = default;

    template <typename U, std::enable_if_t<
        std::is_same_v<std::remove_reference_t<T>, std::remove_reference_t<U>>, int> = 0>
    constexpr maybe(U&& item) : ptr_(std::addressof(item)) {}

//    maybe(T&&) = delete;

    constexpr T& operator*() const & { return *ptr_; }
    constexpr T&& operator*() const && { return std::move(*ptr_); }

    constexpr auto reset() { ptr_ = nullptr; }
    constexpr auto has_value() const -> bool { return ptr_ != nullptr; }
    explicit constexpr operator bool() const { return has_value(); }

    template <typename Func>
    constexpr auto map(Func&& func) const & -> maybe<std::invoke_result_t<Func, T&>>
    {
        if (ptr_) {
            return {FLOW_FWD(func)(**this)};
        }
        return {};
    }

    template <typename Func>
    constexpr auto map(Func&& func) const && -> maybe<std::invoke_result_t<Func, T&&>>
    {
        if (ptr_) {
            return {FLOW_FWD(func)(*std::move(*this))};
        }
        return {};
    }

private:
    T* ptr_;
};

} // namespace flow

#endif


#ifndef FLOW_CORE_COMPARATORS_HPP_INCLUDED
#define FLOW_CORE_COMPARATORS_HPP_INCLUDED




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

inline constexpr auto even = detail::predicate{[](auto const& val) -> bool {
    return val % decltype(val){2} == decltype(val){0};
}};

inline constexpr auto odd = detail::predicate{[](auto const& val) -> bool {
  return val % decltype(val){2} != decltype(val){0};
}};

} // namespaces

#endif


#ifndef FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED
#define FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace flow {

template <typename T>
using remove_cvref_t = std::remove_const_t<std::remove_reference_t<T>>;

template <typename T>
using remove_rref_t = std::conditional_t<
    std::is_rvalue_reference_v<T>, std::remove_reference_t<T>, T>;

template <typename F>
using next_t = decltype(std::declval<F>().next());

template <typename F>
using item_t = typename next_t<F>::value_type;

template <typename F>
using value_t = remove_cvref_t<item_t<F>>;

using dist_t = std::make_signed_t<std::size_t>;

template <typename T>
class maybe;

template <typename Derived>
struct flow_base;

namespace detail {

template <typename>
inline constexpr bool is_maybe = false;

template <typename T>
inline constexpr bool is_maybe<maybe<T>> = true;

template <typename, typename = void>
inline constexpr bool has_next = false;

template <typename T>
inline constexpr bool has_next<T, std::enable_if_t<is_maybe<next_t<T>>>> = true;

} // namespace detail

template <typename I, typename R = std::remove_reference_t<I>>
inline constexpr bool is_flow
    = std::is_base_of_v<flow_base<R>, R> &&
      std::is_convertible_v<std::add_lvalue_reference_t<I>, flow_base<R>&> &&
      detail::has_next<I>;

} // namespace flow

#endif



#ifndef FLOW_SOURCE_FROM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_HPP_INCLUDED

#include <cassert>
#include <iterator>

namespace flow {

namespace detail {

template <typename R>
using iterator_t = decltype(std::begin(std::declval<R&>()));

template <typename R>
using sentinel_t = decltype(std::end(std::declval<R&>()));

template <typename R>
using iter_reference_t = decltype(*std::declval<iterator_t<R>&>());

template <typename R>
using iter_value_t = typename std::iterator_traits<iterator_t<R>>::value_type;

template <typename R>
using iter_category_t =
    typename std::iterator_traits<iterator_t<R>>::iterator_category;

template <typename, typename = void>
inline constexpr bool is_stl_range = false;

template <typename R>
inline constexpr bool is_stl_range<
    R, std::void_t<iterator_t<R>, sentinel_t<R>, iter_category_t<R>>> = true;

template <typename R>
inline constexpr bool is_fwd_stl_range =
    std::is_base_of_v<std::forward_iterator_tag, iter_category_t<R>>;

template <typename R>
inline constexpr bool is_random_access_stl_range =
    std::is_base_of_v<std::random_access_iterator_tag, iter_category_t<R>>;

template <typename R>
struct range_ref {
    constexpr range_ref(R& rng) : ptr_(std::addressof(rng)) {}

    constexpr auto begin() const { return std::begin(*ptr_); }
    constexpr auto end() const { return std::end(*ptr_); }

private:
    R* ptr_;
};

template <typename R>
struct stl_input_range_adaptor : flow_base<stl_input_range_adaptor<R>> {
private:
    R rng_;
    iterator_t<R> cur_ = std::begin(rng_);

public:
    constexpr explicit stl_input_range_adaptor(R&& range)
        : rng_(std::move(range))
    {}

    // We are move-only
    constexpr stl_input_range_adaptor(stl_input_range_adaptor&&) = default;
    constexpr stl_input_range_adaptor&
    operator=(stl_input_range_adaptor&&) = default;

    constexpr auto next() -> maybe<iter_value_t<R>>
    {
        if (cur_ != std::end(rng_)) {
            auto val = *cur_;
            ++cur_;
            return {std::move(val)};
        }
        return {};
    }

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(cur_, std::end(rng_));
        }
    }
};

template <typename R>
struct stl_fwd_range_adaptor : flow_base<stl_fwd_range_adaptor<R>> {
public:
    constexpr explicit stl_fwd_range_adaptor(R&& range) : rng_(std::move(range))
    {}

    constexpr auto next() -> maybe<detail::iter_reference_t<R>>
    {
        if (cur_ != std::end(rng_)) {
            return {*cur_++};
        }
        return {};
    }

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(cur_, std::end(rng_));
        }
    }

private:
    R rng_;
    iterator_t<R> cur_ = std::begin(rng_);
};

template <typename R>
struct stl_ra_range_adaptor : flow_base<stl_ra_range_adaptor<R>> {

    template <typename RR = R, std::enable_if_t<!std::is_array_v<RR>, int> = 0>
    constexpr explicit stl_ra_range_adaptor(R&& rng) : rng_(FLOW_FWD(rng))
    {}

    // Why would anyone ever try to use a raw language array with flow::from?
    // I don't know, but let's try to support them in their foolish quest anyway
    template <typename T, std::size_t N>
    constexpr explicit stl_ra_range_adaptor(T(&&arr)[N])
    {
        for (std::size_t i = 0; i < N; i++) {
            rng_[i] = std::move(arr)[i];
        }
    }

    constexpr auto next() -> maybe<iter_reference_t<R>>
    {
        if (idx_ < std::end(rng_) - std::begin(rng_)) {
            return {std::begin(rng_)[idx_++]};
        }
        return {};
    }

    constexpr auto advance(dist_t dist)
    {
        assert(dist > 0);
        idx_ += dist - 1;
        return next();
    }

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(std::begin(rng_) + idx_, std::end(rng_));
        }
    }

private:
    R rng_{};
    dist_t idx_ = 0;
};

template <typename R, typename = std::enable_if_t<detail::is_stl_range<R>>>
constexpr auto to_flow(R&& range)
{
    using rng_t =
        std::conditional_t<std::is_lvalue_reference_v<R>,
                           detail::range_ref<std::remove_reference_t<R>>, R>;

    if constexpr (detail::is_random_access_stl_range<R>) {
        return detail::stl_ra_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else if constexpr (detail::is_fwd_stl_range<R>) {
        return detail::stl_fwd_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else {
        static_assert(
            !std::is_lvalue_reference_v<R>,
            "Input ranges must be passed as rvalues -- use std::move()");
        return detail::stl_input_range_adaptor<R>{FLOW_FWD(range)};
    }
}

template <typename, typename = void>
inline constexpr bool has_member_to_flow = false;

template <typename T>
inline constexpr bool has_member_to_flow<
    T, std::enable_if_t<is_flow<decltype(std::declval<T>().to_flow())>>> = true;

template <typename, typename = void>
inline constexpr bool has_adl_to_flow = false;

template <typename T>
inline constexpr bool has_adl_to_flow<
    T, std::enable_if_t<is_flow<decltype(to_flow(std::declval<T>()))>>> = true;

} // namespace detail

template <typename T>
inline constexpr bool is_flowable =
    detail::has_member_to_flow<T> || detail::has_adl_to_flow<T>;

namespace detail {

struct from_fn {
    template <typename T, typename = std::enable_if_t<is_flowable<T>>>
    constexpr auto operator()(T&& t) const  -> decltype(auto)
    {
        if constexpr (detail::has_member_to_flow<T>) {
            return FLOW_FWD(t).to_flow();
        } else if constexpr (detail::has_adl_to_flow<T>) {
            return to_flow(FLOW_FWD(t));
        }
    }
};

} // namespace detail

inline constexpr detail::from_fn from{};

template <typename T>
using flow_t = decltype(from(std::declval<T>()));

template <typename T>
using flow_item_t = item_t<flow_t<T>>;

template <typename T>
using flow_value_t = value_t<flow_t<T>>;

} // namespace flow

#endif


#include <cassert>
#include <iosfwd>  // for stream_to()
#include <string>  // for to_string()
#include <vector>  // for to_vector()

namespace flow {

template <typename Derived>
struct flow_base {
private:
    constexpr auto derived() & -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto consume() -> Derived&& { return static_cast<Derived&&>(*this); }

    friend constexpr auto to_flow(flow_base& self) -> Derived& { return static_cast<Derived&>(self); }
    friend constexpr auto to_flow(flow_base const& self) -> Derived const& { return static_cast<Derived const&>(self); }
    friend constexpr auto to_flow(flow_base&& self) { return self.consume(); }

protected:
    ~flow_base() = default;

public:
    template <typename Adaptor, typename... Args>
    constexpr auto apply(Adaptor&& adaptor, Args&&... args) && -> decltype(auto)
    {
        return invoke(FLOW_FWD(adaptor), consume(), FLOW_FWD(args)...);
    }

    template <typename D = Derived>
    constexpr auto advance(dist_t dist) -> next_t<D>
    {
        assert(dist > 0);
        for (dist_t i = 0; i < dist - 1; i++) {
            derived().next();
        }

        return derived().next();
    }

    /// Short-circuiting left fold operation.
    ///
    /// Given a function `func` and an initial value `init`, repeatedly calls
    /// `func(std::move(init), next())` and assigns the result to `init`. If
    /// `init` then evaluates to `false`, it immediately exits, returning
    /// the accumulated value. For an empty flow, it simply returns the initial
    /// value.
    ///
    /// Note that the type of the second parameter to `func` differs from plain
    /// `fold()`: this version takes a `maybe`, which `fold()` unwraps for
    /// convenience.
    ///
    /// Because this function is short-circuiting, the flow may be restarted
    /// after it has returned and the remaining items (if any) may be processed.
    ///
    /// This is the lowest-level reduction operation, on which all the other
    /// reductions are (eventually) based. For most user code, one of the
    /// higher-level operations is usually more convenient.
    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init;

    /// Short-circuiting version of for_each().
    ///
    /// Given a unary function `func`, repeatedly calls `func(next())`. If the
    /// return value of the function evaluates to `false`, it immediately exits,
    /// providing the last function return value.
    ///
    /// `try_for_each()` requires that the return type of `func` is "bool-ish":
    /// that it is default-constructible, move-assignable, and contextually
    /// convertible to `bool`.
    ///
    /// Note that the type of the parameter to `func` differs from plain
    /// `for_each()`: this version takes a `maybe`, which `for_each()` unwraps for
    /// convenience.
    ///
    /// Because this function is short-circuiting, the flow may be restarted
    /// after it has returned and the remaining items (if any) may be processed.
    ///
    /// This is a low-level operation, effectively a stateless version of
    /// `try_fold()`. For most user code, higher-level functions such as
    /// `for_each()` will be more convenient.
    template <typename Func>
    constexpr auto try_for_each(Func func);

    /// Consumes the flow, performing a functional left fold operation.
    ///
    /// Given a callable `func` and an initial value `init`, calls
    /// `init = func(std::move(init), i)` for each item `i` in the flow. Once
    /// the flow is exhausted, it returns the value accumulated in `init`.
    ///
    /// This is the same operation as `std::accumulate`.
    ///
    /// @func Callable with signature `(Init, item_t<Flow>) -> Init`
    /// @init Initial value of the reduction
    /// @returns The accumulated value
    template <typename Func, typename Init>
    constexpr auto fold(Func func, Init init) -> Init;

    /// Consumes the flow, performing a functional left fold operation.
    ///
    /// This is a convenience overload, equivalent to `fold(func, value_t{})`
    /// where `value_t` is the value type of the flow.
    template <typename Func>
    constexpr auto fold(Func func);

    /// Consumes the flow, applying the given function to each element
    ///
    /// @func A unary callable accepting this flow's item type
    /// @returns The supplied function
    template <typename Func>
    constexpr auto for_each(Func func) -> Func;

    /// Consumes the flow, returning the number of items for which `pred`
    /// returned true
    ///
    /// Equivalent to `filter(pred).count()`.
    ///
    /// @pred A predicate accepting the flow's item type
    /// @returns The number of items for which the `pred(item)` returned true
    template <typename Pred>
    constexpr auto count_if(Pred pred) -> dist_t;

    /// Consumes the flow, returning the number of items it contained
    constexpr auto count() -> dist_t;

    /// Consumes the flow, returning a count of the number of items which
    /// compared equal to `value`, using comparator `cmp`
    ///
    /// @value Value which each item in the flow should be compared to
    /// @cmp The comparator to be used (default is std::equal<>)
    /// @returns The number of items for which `cmp(value, item)` returned true
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto count(const T& value, Cmp cmp = {}) -> dist_t;

    /// Processes the flow until it finds an item that compares equal to
    /// `value`, using comparator `cmp`.
    ///
    /// If the item is not found, returns an empty `maybe`.
    ///
    /// Unlike most operations, this function is short-circuiting: it will
    /// return immediately after finding an item, after which the flow can be
    /// restarted and the remaining items (if any) can be processed.
    ///
    /// @value Value to find
    /// @cmp Comparator to use for the find operation (default: `std::equal<>`)
    /// @returns A `maybe` containing the first item for which `cmp(value, item)`
    ///          returned true, or an empty `maybe` if the flow was consumed without
    ///          finding such an item.
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto find(const T& value, Cmp cmp = {});

    /// Returns `true` if the flow contains an item which compares equal to
    /// `value`, using comparator `cmp`.
    ///
    /// This is a convenience method, equivalent to
    /// `static_cast<bool>(my_flow.find(value, cmp))`.
    ///
    /// Unlike most operations, this function is short-circuiting: it will
    /// return immediately after finding an item, after which the flow can be
    /// restarted and the remaining items (if any) can be processed.
    ///
    /// @value Value to find
    /// @cmp Comparator to use, defaults to `std::equal<>`
    /// @return true iff the flow contained an item for which `cmp(value, item)`
    ///          returned true.
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto contains(const T& value, Cmp cmp = {}) -> bool;

    /// Consumes the flow, returning the sum of items using `operator+`
    ///
    /// Requires that the flow's value type is default-constructible.
    ///
    /// This is a convenience method, equivalent to `fold(std::plus<>{})`
    constexpr auto sum();

    /// Consumes the flow, returning the product of the elements using `operator*`
    ///
    /// Requires that the flow's value type is constructible from a literal `1`
    constexpr auto product();

    /// Consumes the flow, returning the smallest item according `cmp`
    ///
    /// If several items are equally minimal, returns the first. If the flow
    /// is empty, returns an empty `maybe`.
    template <typename Cmp = std::less<>>
    constexpr auto min(Cmp cmp = Cmp{});

    /// Consumes the flow, returning the largest item according to `cmp`
    ///
    /// If several items are equal maximal, returns the last. If the flow
    /// is empty, returns an empty `maybe`.
    template <typename Cmp = std::less<>>
    constexpr auto max(Cmp cmp = Cmp{});

    /// Consumes the flow, returning both the minimum and maximum values
    /// according to `cmp`.
    ///
    /// If several items are equal minimal, returns the first. If several items
    /// are equally maximal, returns the last. If the flow is empty, returns
    /// an empty `maybe`.
    template <typename Cmp = std::less<>>
    constexpr auto minmax(Cmp cmp = Cmp{});

    /// Processes the flow, returning true if all the items satisfy the predicate.
    /// Returns `true` for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when an item fails to satisfy the predicate.
    template <typename Pred>
    constexpr auto all(Pred pred) -> bool;

    /// Processes the flow, returning false if any element satisfies the predicate.
    /// Returns `true` for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when an item satisfies the predicate.
    template <typename Pred>
    constexpr auto none(Pred pred) -> bool;

    /// Processes the flow, returning true if any element satisfies the predicate.
    /// Returns `false` for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when an item satisfies the predicate.
    template <typename Pred>
    constexpr auto any(Pred pred) -> bool;

    /// Consumes the flow, returning true if the elements are sorted according to
    /// the given comparator, defaulting to std::less<>.
    ///
    /// Returns true for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when it finds an item which is not in sorted order.
    template <typename Cmp = std::less<>>
    constexpr auto is_sorted(Cmp cmp = Cmp{}) -> bool;

    template <typename NextFn>
    constexpr auto adapt(NextFn next_fn) &&;

private:
    template <typename Func>
    struct map_adaptor : flow_base<map_adaptor<Func>> {
        constexpr map_adaptor(Derived&& self, Func&& func)
            : self_(std::move(self)),
              func_(std::move(func))
        {}

        constexpr auto next() -> maybe<std::invoke_result_t<Func&, item_t<Derived>>>
        {
            return self_.next().map(func_);
        }

        Derived self_;
        FLOW_NO_UNIQUE_ADDRESS Func func_;
    };

public:
    template <typename Func>
    constexpr auto map(Func func) && -> map_adaptor<Func>
    {
        static_assert(std::is_invocable_v<Func&, item_t<Derived>&&>,
            "Incompatible callable passed to map()");
        static_assert(!std::is_void_v<std::invoke_result_t<Func&, item_t<Derived>&&>>,
            "Map cannot be used with a function returning void");
        return map_adaptor<Func>(consume(), std::move(func));
    }

    template <typename = Derived>
    constexpr auto deref() &&
    {
        auto deref = [](auto&& val) -> decltype(auto) { return *val; };
        static_assert(std::is_invocable_v<decltype(deref), item_t<Derived>>,
            "Flow's item type is not dereferenceable with unary operator*");
        static_assert(!std::is_void_v<decltype(*std::declval<item_t<Derived>>())>,
            "Flow's item type dereferences to void");
        return consume().map(deref);
    }

    template <typename T>
    constexpr auto as() &&
    {
        return consume().map([](auto&& val) -> decltype(auto) {
            return static_cast<T>(FLOW_FWD(val));
        });
    }

    constexpr auto as_const() && -> decltype(auto)
    {
        if constexpr (std::is_lvalue_reference_v<item_t<Derived>>) {
            return consume().template as<value_t<Derived> const&>();
        } else {
            return consume();
        }
    }

    constexpr auto copy() && -> decltype(auto)
    {
        if constexpr (std::is_lvalue_reference_v<item_t<Derived>>) {
            return consume().template as<value_t<Derived>>();
        } else {
            return consume();
        }
    }

    constexpr auto move() && -> decltype(auto)
    {
        if constexpr (std::is_lvalue_reference_v<item_t<Derived>>) {
            return consume()
                .template as<std::remove_reference_t<item_t<Derived>>&&>();
        } else {
            return consume();
        }
    }

    /// Consumes the flow, returning a new flow which yields the same items
    /// but calls @func at each call to `next()`, passing it a const reference
    /// to the item.
    ///
    /// `inspect()` is intended as an aid to debugging. It can be
    /// inserted at any point in a pipeline, and allows examining the items
    /// in the flow at that point (for example, to print them).
    template <typename Func>
    constexpr auto inspect(Func func) &&;

    template <typename Pred>
    constexpr auto filter(Pred pred) &&
    {
        static_assert(std::is_invocable_v<Pred&, item_t<Derived>&>,
            "Incompatible predicate passed to filter()");
        constexpr auto is_boolish = [](auto&& p) { return p ? true : false; };
        static_assert(std::is_invocable_v<decltype(is_boolish),
            std::invoke_result_t<Pred&, item_t<Derived>&>>,
            "filter predicate must return a type which is contextutally convertible to bool");

        auto filter_fn = [self = consume(), pred = std::move(pred)] () mutable
            -> maybe<item_t<Derived>>
            {
                while (auto o = self.next()) {
                    if (invoke(pred, *o)) {
                        return {std::move(o)};
                    }
                }
                return {};
            };
        return consume().adapt(std::move(filter_fn));
    }

    template <typename Func>
    constexpr auto filter_map(Func func) &&
    {
        static_assert(std::is_invocable_v<Func&, item_t<Derived>>);
        auto to_bool = [](auto&& a) { return static_cast<bool>(a); };
        using ret_t = std::invoke_result_t<Func&, item_t<Derived>>;
        static_assert(std::is_invocable_v<decltype(to_bool), ret_t&>);

        return consume().map(std::move(func)).filter([] (auto&& val) {
            return val ? true : false;
        }).deref();
    }

    template <typename = Derived>
    constexpr auto drop(dist_t count) &&
    {
        auto drop_fn = [self = consume(), count = count] () mutable {
            if (count > 0) {
                self.advance(count);
                count = 0;
            }
            return self.next();
        };
        return consume().adapt(std::move(drop_fn));
    }

    template <typename Pred>
    constexpr auto drop_while(Pred pred) &&
    {
        auto fn = [self = consume(),
                   pred = std::move(pred),
                   done = false] () mutable -> maybe<item_t<Derived>>
        {
            while (auto m = self.next()) {
                if (!done) {
                    if (invoke(pred, *m)) {
                        continue;
                    } else {
                        done = true;
                    }
                }
                return m;
            }
            return {};
        };
        return consume().adapt(std::move(fn));
    }

    struct take_adaptor : flow_base<take_adaptor>
    {
        constexpr take_adaptor(Derived&& derived, dist_t count)
            : derived_(std::move(derived)), count_(count)
        {}

        constexpr auto next() -> maybe<item_t<Derived>>
        {
            if (count_ > 0) {
                --count_;
                return derived_.next();
            }
            return {};
        }

    private:
        Derived derived_;
        dist_t count_;
    };

    template <typename = Derived>
    constexpr auto take(dist_t count) &&
    {
        return take_adaptor{consume(), count};
    }

    template <typename Pred>
    constexpr auto take_while(Pred pred) &&
    {
        auto fn = [self = consume(),
                   pred = std::move(pred),
                   done = false] () mutable -> maybe<item_t<Derived>> {
            if (!done) {
                auto m = self.next();
                if (invoke(pred, *m)) {
                    return m;
                } else {
                    done = true;
                }
            }
            return {};
        };
        return consume().adapt(std::move(fn));
    }

    template <typename = Derived>
    constexpr auto step_by(dist_t stride) &&
    {
        auto fn = [self = consume(), stride = stride, first = true] () mutable
        {
            if (first) {
                first = false;
                return self.next();
            }
            return self.advance(stride);
        };
        return consume().adapt(std::move(fn));
    }

    struct cycle_adaptor : flow_base<cycle_adaptor>
    {
        constexpr cycle_adaptor(Derived&& d)
            : init_(std::move(d))
        {}

        constexpr auto next() -> next_t<Derived>
        {
            while (true) {
                if (auto m = cur_.next()) {
                    return m;
                }
                cur_ = init_;
            }
        }

    private:
        Derived init_;
        Derived cur_ = init_;
    };

    template <typename = Derived>
    constexpr auto cycle() &&
    {
        return cycle_adaptor{consume()};
    }

private:
    template <typename Other>
    struct chain_adaptor : flow_base<chain_adaptor<Other>> {

        constexpr chain_adaptor(Derived&& self, Other&& other)
            : self_(std::move(self)),
              other_(std::move(other))
        {}

        constexpr auto next() -> next_t<Derived>
        {
            if (first_) {
                if (auto m = self_.next()) {
                    return m;
                }
                first_ = false;
            }

            return other_.next();
        }

    private:
        Derived self_;
        Other other_;
        bool first_ = true;
   };

public:
    template <typename Flow>
    constexpr auto chain(Flow other) &&
    {
        static_assert(std::is_same_v<item_t<Derived>, item_t<Flow>>,
            "Flows used with chain() must have the exact same item type");

        return chain_adaptor<Flow>(consume(), std::move(other));
    }

    template <typename Flow>
    constexpr auto interleave(Flow with) &&;

    constexpr auto flatten() &&;

    template <typename Func>
    constexpr auto flat_map(Func func) &&;

    template <typename... Flowables>
    constexpr auto zip(Flowables&&... flowables) &&
    {
        static_assert((is_flowable<Flowables> && ...),
                      "Arguments to zip() must be Flowable types");

        using item_type = std::conditional_t<
            sizeof...(Flowables) == 1,
             std::pair<item_t<Derived>, item_t<flow_t<Flowables>>...>,
             std::tuple<item_t<Derived>, item_t<flow_t<Flowables>>...>>;

        return consume().zip_with([](auto&& v1, auto&& v2) {
            return item_type{FLOW_FWD(v1), FLOW_FWD(v2)};
        }, FLOW_FWD(flowables)...);
    }

    template <typename Func, typename... Flowables>
    constexpr auto zip_with(Func func, Flowables&&... flowables) &&;

    constexpr auto enumerate() &&;

    template <typename Key>
    constexpr auto group_by(Key func) &&;

    template <typename D = Derived>
    constexpr auto split(value_t<D> delimiter) &&
    {
        return consume().group_by(flow::pred::eq(std::move(delimiter))).step_by(2);
    }

    /// Consumes the flow, returning a new flow where each item is a flow
    /// containing `size` items. The last chunk may have fewer items.
    ///
    /// Note that, to avoid copies or allocations, each inner flow is invalidated
    /// once the parent flow is advanced. In other words, the inner flows must
    /// be processed in order.
    constexpr auto chunk(dist_t size) &&;

    template <std::size_t N>
    constexpr auto elements() &&
    {
        return consume().map(
            [] (auto&& val) -> remove_rref_t<decltype(std::get<N>(FLOW_FWD(val)))> {
            return std::get<N>(FLOW_FWD(val));
        });
    }

    template <typename = Derived>
    constexpr auto keys() &&
    {
        return consume().template elements<0>();
    }

    template <typename = Derived>
    constexpr auto values() &&
    {
        return consume().template elements<1>();
    }

    template <typename Flowable>
    constexpr auto equal(Flowable&& flowable) && -> bool
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to equal() must be a Flowable type");

        auto&& other = flow::from(FLOW_FWD(flowable));

        while (true) {
            auto m1 = derived().next();
            auto m2 = other.next();

            if (!m1) {
                return !static_cast<bool>(m2);
            }
            if (!m2) {
                return false;
            }

            if (*m1 != *m2) {
                return false;
            }
        }
    }

    constexpr auto to_range() &&;

    template <typename C>
    constexpr auto to() &&
    {
        auto rng = consume().to_range();
        static_assert(std::is_constructible_v<C, decltype(rng.begin()), decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }

    template <template <typename...> typename C>
    constexpr auto to() &&
    {
        auto rng = consume().to_range();
        return C(rng.begin(), rng.end());
    }

    template <typename = Derived>
    constexpr auto collect() &&;

    constexpr auto to_vector() &&
    {
        return consume().template to<std::vector<value_t<Derived>>>();
    }

    template <typename T>
    constexpr auto to_vector() && -> std::vector<T>
    {
        return consume().template to<std::vector<T>>();
    }

    template <typename = Derived>
    constexpr auto to_string() &&
    {
        return consume().template to<std::string>();
    }

    template <typename Iter>
    constexpr auto output_to(Iter oiter) && -> Iter
    {
        consume().for_each([&oiter] (auto&& val) {
            *oiter = FLOW_FWD(val);
            ++oiter;
        });
        return oiter;
    }

    template <typename Sep = const char*>
    constexpr auto write_to(std::ostream& os, Sep sep = ", ") &&
    {
        consume().for_each([&os, &sep, first = true](auto&& m) mutable {
            if (first) {
                first = false;
            } else {
                os << sep;
            }
            os << FLOW_FWD(m);
        });
    }
};

}

#endif




#ifndef FLOW_OP_ADAPT_HPP_INCLUDED
#define FLOW_OP_ADAPT_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename NextFn>
struct adaptor : flow_base<adaptor<NextFn>> {

    constexpr explicit adaptor(NextFn&& next_fn) : next_fn_(std::move(next_fn))
    {}

    constexpr auto next() { return next_fn_(); }

private:
    FLOW_NO_UNIQUE_ADDRESS NextFn next_fn_;
};

}

template <typename D>
template <typename NextFn>
constexpr auto flow_base<D>::adapt(NextFn next_fn) &&
{
    return detail::adaptor<NextFn>(std::move(next_fn));
}

}

#endif


#ifndef FLOW_OP_ALL_ANY_NONE_HPP_INCLUDED
#define FLOW_OP_ALL_ANY_NONE_HPP_INCLUDED



namespace flow {

inline constexpr auto all = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::all() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).all(std::move(pred));
};

inline constexpr auto any = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::any() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).any(std::move(pred));
};

inline constexpr auto none = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::none() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).none(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::all(Pred pred) -> bool
{
    static_assert(std::is_invocable_r_v<bool, Pred&, item_t<D>>,
                  "Predicate must be callable with the Flow's item_type,"
                   " and must return bool");
    return derived().try_fold([&pred](bool /*unused*/, auto&& m) {
      return invoke(pred, *FLOW_FWD(m));
    }, true);
}

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::any(Pred pred) -> bool
{
    return !derived().none(std::move(pred));
}

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::none(Pred pred) -> bool
{
    return derived().all(pred::not_(std::move(pred)));
}

}

#endif


#ifndef FLOW_OP_CHUNK_HPP_INCLUDED
#define FLOW_OP_CHUNK_HPP_INCLUDED



namespace flow {

namespace detail {

struct chunk_counter {

    explicit constexpr chunk_counter(dist_t size)
        : size_(size)
    {}

    template <typename T>
    constexpr auto operator()(T&& /*unused*/) -> bool
    {
        if (++counter_ < size_) {
            return last_;
        }
        counter_ = 0;
        return (last_ = !last_);
    }

private:
    const dist_t size_;
    dist_t counter_ = -1; // Begin off-track
    bool last_ = true;
};

}

template <typename Derived>
constexpr auto flow_base<Derived>::chunk(dist_t size) &&
{
    assert((size > 0) && "Chunk size must be greater than zero");
    return consume().group_by(detail::chunk_counter{size});
}

}

#endif


#ifndef FLOW_OP_COLLECT_HPP_INCLUDED
#define FLOW_OP_COLLECT_HPP_INCLUDED



#ifndef FLOW_OP_TO_RANGE_HPP_INCLUDED
#define FLOW_OP_TO_RANGE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename F>
struct flow_range {
private:
    struct iterator {
        using value_type = value_t<F>;
        using reference = item_t<F>;
        using difference_type = dist_t;
        using pointer = value_type*;
        using iterator_category = std::input_iterator_tag;

        constexpr iterator() = default;

        constexpr explicit iterator(flow_range& parent)
            : parent_(std::addressof(parent))
        {}

        constexpr iterator& operator++()
        {
            if (parent_) {
                if (!parent_->do_next()) {
                    parent_ = nullptr;
                }
            }
            return *this;
        }

        constexpr reference operator*() const
        {
            return *std::move(parent_->item_);
        }

        friend constexpr bool operator==(const iterator& lhs,
                                         const iterator& rhs)
        {
            return lhs.parent_ == rhs.parent_;
        }

        friend constexpr bool operator!=(const iterator& lhs,
                                         const iterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        flow_range* parent_ = nullptr;
    };

public:
    constexpr explicit flow_range(F&& flow)
        : flow_(std::move(flow)), item_(flow_.next())
    {}

    constexpr auto begin() { return iterator{*this}; }
    constexpr auto end() { return iterator{}; }

private:
    bool do_next()
    {
        item_ = flow_.next();
        return (bool) item_;
    }

    F flow_;
    maybe<item_t<F>> item_;
};

}

template <typename D>
constexpr auto flow_base<D>::to_range() &&
{
    return detail::flow_range<D>{consume()};
}

}

#endif

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


#ifndef FLOW_OP_CONTAINS_HPP_INCLUDED
#define FLOW_OP_CONTAINS_HPP_INCLUDED


#ifndef FLOW_OP_FIND_HPP_INCLUDED
#define FLOW_OP_FIND_HPP_INCLUDED



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


namespace flow {

namespace detail {

struct contains_op {
    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& item, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::contains() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).contains(item, std::move(cmp));
    }
};

}

inline constexpr auto contains = detail::contains_op{};

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

}

#endif


#ifndef FLOW_OP_COUNT_HPP_INCLUDED
#define FLOW_OP_COUNT_HPP_INCLUDED


#ifndef FLOW_OP_COUNT_IF_HPP_INCLUDED
#define FLOW_OP_COUNT_IF_HPP_INCLUDED


#ifndef FLOW_OP_FOLD_HPP_INCLUDED
#define FLOW_OP_FOLD_HPP_INCLUDED



namespace flow {

namespace detail {

struct fold_op {

    template <typename Flowable, typename Func, typename Init>
    constexpr auto operator()(Flowable&& flowable, Func func, Init init) const
    {
        static_assert(is_flowable<Flowable>,
                "First argument to flow::fold() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold(std::move(func), std::move(init));
    }

    template <typename Flowable, typename Func>
    constexpr auto operator()(Flowable&& flowable, Func func) const
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::fold() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold(std::move(func));
    }
};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};

template <typename Derived>
template <typename Func, typename Init>
constexpr auto flow_base<Derived>::fold(Func func, Init init) -> Init
{
    static_assert(std::is_invocable_v<Func&, Init&&, item_t<Derived>>,
                  "Incompatible callable passed to fold()");
    static_assert(std::is_assignable_v<Init&, std::invoke_result_t<Func&, Init&&, item_t<Derived>>>,
                  "Accumulator of fold() is not assignable from the result of the function");

    struct always {
        Init val;
        constexpr explicit operator bool() const { return true; }
    };

    return derived().try_fold([&func](always acc, auto m) {
        return always{func(std::move(acc).val, *std::move(m))};
    }, always{std::move(init)}).val;
}

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::fold(Func func)
{
    static_assert(std::is_default_constructible_v<value_t<Derived>>,
            "This flow's value type is not default constructible. "
            "Use the fold(function, initial_value) overload instead");
    return consume().fold(std::move(func), value_t<Derived>{});
}

}

#endif


namespace flow {

inline constexpr auto count_if = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
        "First argument to flow::count_if must be Flowable");
    return flow::from(FLOW_FWD(flowable)).count_if(std::move(pred));
};

template <typename Derived>
template <typename Pred>
constexpr auto flow_base<Derived>::count_if(Pred pred) -> dist_t
{
    static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                  "Predicate must be callable with the Flow's item_type,"
                  " and must return bool");
    return consume().fold([&pred](dist_t count, auto&& val) {
      return count + static_cast<dist_t>(invoke(pred, FLOW_FWD(val)));
    }, dist_t{0});
}

} // namespace flow

#endif


namespace flow {

namespace detail {

struct count_op {

    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable) const
    {
        static_assert(is_flow<Flowable>,
            "Argument to flow::count() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).count();
    }

    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& value, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::count() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).count(value, std::move(cmp));
    }

};

} // namespace detail

inline constexpr auto count = detail::count_op{};

template <typename Derived>
constexpr auto flow_base<Derived>::count() -> dist_t
{
    return consume().count_if([](auto const& /*unused*/) {
        return true;
    });
}

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::count(const T& item, Cmp cmp) -> dist_t
{
    using const_item_t = std::add_lvalue_reference_t<
        std::add_const_t<std::remove_reference_t<item_t<Derived>>>>;
    static_assert(std::is_invocable_r_v<bool, Cmp&, const T&, const_item_t>,
        "Incompatible comparator used with count()");

    return consume().count_if([&item, &cmp] (auto const& val) {
        return invoke(cmp, item, val);
    });
}

}

#endif




#ifndef FLOW_OP_FLATTEN_HPP_INCLUDED
#define FLOW_OP_FLATTEN_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Base>
struct flatten_adaptor : flow_base<flatten_adaptor<Base>> {

    constexpr explicit flatten_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    constexpr auto next() -> next_t<flow_t<item_t<Base>>>
    {
        while (true) {
            if (!inner_) {
                inner_ = base_.next().map(flow::from);
                if (!inner_) {
                    return {};
                }
            }

            if (auto m = inner_->next()) {
                return m;
            } else {
                inner_ = {};
            }
        }
    }

private:
    Base base_;
    maybe<flow_t<item_t<Base>>> inner_{};
};

}

template <typename Derived>
constexpr auto flow_base<Derived>::flatten() &&
{
    static_assert(is_flowable<value_t<Derived>>,
        "flatten() requires a flow whose item type is flowable");
    return detail::flatten_adaptor<Derived>(consume());
}

}

#endif


#ifndef FLOW_OP_FLAT_MAP_HPP_INCLUDED
#define FLOW_OP_FLAT_MAP_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Base, typename Func>
struct flat_map_adaptor : flow_base<flat_map_adaptor<Base, Func>> {

    using mapped_type = std::invoke_result_t<Func&, item_t<Base>>;
    using item_type = item_t<mapped_type>;

    constexpr flat_map_adaptor(Base&& base, Func&& func)
        : base_(std::move(base)),
          func_(std::move(func))
    {}

    constexpr auto next() -> maybe<item_type>
    {
        while (true) {
            if (!inner_) {
                inner_ = base_.next().map(func_);
                if (!inner_) {
                    return {};
                }
            }

            if (auto m = inner_->next()) {
                return std::move(m);
            } else {
                inner_.reset();
            }
        }
    }

private:
    Base base_;
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    maybe<mapped_type> inner_;
};

} // namespace detail

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::flat_map(Func func) &&
{
    return detail::flat_map_adaptor<Derived, Func>(consume(), std::move(func));
}

} // namespace flow

#endif



#ifndef FLOW_OP_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_FOR_EACH_HPP_INCLUDED



namespace flow {

inline constexpr auto for_each = [](auto flowable, auto func) {
    static_assert(is_flowable<decltype(flowable)>,
        "First argument to flow::for_each() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).for_each(std::move(func));
};

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::for_each(Func func) -> Func
{
    static_assert(std::is_invocable_v<Func&, item_t<Derived>>,
                  "Incompatible callable passed to for_each()");
    consume().fold([&func](bool, auto&& val) {
      (void) func(FLOW_FWD(val));
      return true;
    }, true);
    return func;
}

}

#endif



#ifndef FLOW_OP_GROUP_BY_HPP_INCLUDED
#define FLOW_OP_GROUP_BY_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Base, typename Key>
struct group_by_adaptor : flow_base<group_by_adaptor<Base, Key>> {
private:

    using key_type = std::invoke_result_t<Key&, item_t<Base>&>;

    Base base_;
    FLOW_NO_UNIQUE_ADDRESS Key key_;
    next_t<Base> next_item_{};
    maybe<key_type> last_key_{};
    bool group_exhausted_ = false;

    struct group : flow_base<group>
    {
        constexpr explicit group(group_by_adaptor& parent)
            : parent_(std::addressof(parent))
        {}

        constexpr group(group&&) noexcept = default;
        constexpr group& operator=(group&&) noexcept = default;

        constexpr auto next() -> next_t<Base>
        {
            if (parent_->group_exhausted_) {
                return {};
            }

            if (!parent_->next_item_) {
                if (!parent_->do_next()) {
                    parent_->group_exhausted_ = true;
                    return {};
                }
            }

            auto item = std::move(parent_->next_item_);
            parent_->next_item_.reset();

            return item;
        }

    private:
        group_by_adaptor* parent_;
    };

    constexpr auto do_next() -> bool
    {
        next_item_ = base_.next();
        if (!next_item_) {
            // We are done
            return false;
        }

        auto&& next_key = invoke(key_, *next_item_);
        if (next_key != *last_key_) {
            last_key_ = FLOW_FWD(next_key);
            return false;
        }

        return true;
    }

public:
    constexpr group_by_adaptor(Base&& base, Key&& key)
        : base_(std::move(base)), key_(std::move(key))
    {}

    constexpr auto next() -> maybe<group>
    {
        // Is this the first time we're being called?
        if (!last_key_) {
            next_item_ = base_.next();
            if (next_item_) {
                last_key_ = invoke(key_, *next_item_);
                return group{*this};
            } else {
                return {};
            }
        }

        // If the last group was exhausted, go ahead and start a new group
        if (group_exhausted_) {
            // Did we run out of items?
            if (!next_item_) {
                return {};
            }

            group_exhausted_ = false;
            return group{*this};
        }

        // Last group was not exhausted: fast-forward to the next group
        while (do_next())
            ;
        if (!next_item_) {
            return {};
        }
        return group{*this};

    }
};

}

template <typename Derived>
template <typename Key>
constexpr auto flow_base<Derived>::group_by(Key key) &&
{
    return detail::group_by_adaptor<Derived, Key>(consume(), std::move(key));
}

}

#endif


#ifndef FLOW_OP_INSPECT_HPP_INCLUDED
#define FLOW_OP_INSPECT_HPP_INCLUDED



namespace flow {

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::inspect(Func func) &&
{
    static_assert(std::is_invocable_v<Func&, value_t<Derived> const&>,
        "Incompatible callable used with inspect()");

    return consume().map([func = std::move(func)] (auto&& val) mutable -> item_t<Derived> {
          (void) invoke(func, std::as_const(val));
          return FLOW_FWD(val);
    });
}

}

#endif


#ifndef FLOW_OP_INTERLEAVE_HPP_INCLUDED
#define FLOW_OP_INTERLEAVE_HPP_INCLUDED



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

#ifndef FLOW_OP_IS_SORTED_HPP_INCLUDED
#define FLOW_OP_IS_SORTED_HPP_INCLUDED



namespace flow {

namespace detail {

struct is_sorted_fn {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::is_sorted() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).is_sorted(std::move(cmp));
    }
};

}

inline constexpr auto is_sorted = detail::is_sorted_fn{};

template <typename D>
template <typename Cmp>
constexpr bool flow_base<D>::is_sorted(Cmp cmp)
{
    auto last = derived().next();
    if (!last) {
        // An empty flow is sorted
        return true;
    }

    return derived().try_fold([&cmp, &last] (bool, next_t<D>&& next) {
        if (invoke(cmp, *next, *last)) {
            return false;
        } else {
            last = std::move(next);
            return true;
        }
    }, true);
}

}

#endif


#ifndef FLOW_OP_MINMAX_HPP_INCLUDED
#define FLOW_OP_MINMAX_HPP_INCLUDED



namespace flow {

namespace detail {

struct min_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::min() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).min(std::move(cmp));
    }
};

struct max_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::max() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).max(std::move(cmp));
    }
};

struct minmax_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::minmax() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).minmax(std::move(cmp));
    }
};

}

inline constexpr auto min = detail::min_op{};
inline constexpr auto max = detail::max_op{};
inline constexpr auto minmax = detail::minmax_op{};

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::min(Cmp cmp)
{
    // Yes, this is crazy. Sorrynotsorry.
    return derived().next().map([this, &cmp] (auto&& init) {
        return *derived().fold([&cmp](auto init, auto&& item) -> next_t<D> {
            return invoke(cmp, item, *init) ? next_t<D>{FLOW_FWD(item)} : std::move(init);
        }, next_t<D>(FLOW_FWD(init)));
    });
}

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::max(Cmp cmp)
{
    return derived().next().map([this, &cmp] (auto&& init) {
      return *derived().fold([&cmp](auto init, auto&& item) -> next_t<D> {
        return !invoke(cmp, item, *init) ? next_t<D>{FLOW_FWD(item)} : std::move(init);
      }, next_t<D>(FLOW_FWD(init)));
    });
}

template <typename V>
struct minmax_result {
    V min;
    V max;

    friend constexpr bool operator==(const minmax_result& lhs, const minmax_result& rhs)
    {
        return lhs.min == rhs.min && lhs.max == rhs.max;
    }

    friend constexpr bool operator!=(const minmax_result& lhs, const minmax_result& rhs)
    {
        return !(lhs == rhs);
    }
};

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::minmax(Cmp cmp)
{
    return derived().next().map([this, &cmp] (auto&& init) {
        return derived().fold([&cmp](auto mm, auto&& item) -> minmax_result<value_t<D>> {

            if (invoke(cmp, item, mm.min)) {
                mm.min = item;
            }

            if (!invoke(cmp, item, mm.max)) {
                mm.max = FLOW_FWD(item);
            }

            return mm;
        }, minmax_result<value_t<D>>{init, FLOW_FWD(init)});
    });
}

}

#endif


#ifndef FLOW_OP_PRODUCT_HPP_INCLUDED
#define FLOW_OP_PRODUCT_HPP_INCLUDED



namespace flow {

inline constexpr auto product = [](auto&& flowable)
{
  static_assert(is_flowable<decltype(flowable)>,
                "Argument to flow::sum() must be Flowable");
  return flow::from(FLOW_FWD(flowable)).product();
};

template <typename D>
constexpr auto flow_base<D>::product()
{
    static_assert(std::is_constructible_v<value_t<D>, int>,
                  "Flow's value type must be constructible from a literal 1");
    return derived().fold(std::multiplies<>{}, value_t<D>{1});
}

}

#endif


#ifndef FLOW_OP_SUM_HPP_INCLUDED
#define FLOW_OP_SUM_HPP_INCLUDED



namespace flow {

inline constexpr auto sum = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::sum() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).sum();
};

template <typename D>
constexpr auto flow_base<D>::sum()
{
    return derived().fold(std::plus<>{});
}

}

#endif



#ifndef FLOW_OP_TRY_FOLD_HPP_INCLUDED
#define FLOW_OP_TRY_FOLD_HPP_INCLUDED



namespace flow {

inline constexpr auto try_fold = [](auto&& flowable, auto func, auto init) {
    static_assert(is_flowable<decltype(flowable)>);
    return flow::from(FLOW_FWD(flowable)).try_fold(std::move(func), std::move(init));
};

template <typename D>
template <typename Func, typename Init>
constexpr auto flow_base<D>::try_fold(Func func, Init init) -> Init
{
    static_assert(std::is_invocable_r_v<Init, Func&, Init&&, next_t<D>&&>,
                  "Incompatible callable passed to try_fold()");
    while (auto m = derived().next()) {
        init = invoke(func, std::move(init), std::move(m));
        if (!static_cast<bool>(init)) {
            break;
        }
    }
    return init;
}

}

#endif


#ifndef FLOW_OP_TRY_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_TRY_FOR_EACH_HPP_INCLUDED



namespace flow {

inline constexpr auto try_for_each = [](auto flowable, auto func) {
    static_assert(is_flowable<decltype(flowable)>);
    return flow::from(FLOW_FWD(flowable), std::move(func));
};

template <typename D>
template <typename Func>
constexpr auto flow_base<D>::try_for_each(Func func)
{
    using result_t = std::invoke_result_t<Func&, next_t<D>&&>;
    return derived().try_fold([&func](auto&& /*unused*/, auto&& m) -> decltype(auto) {
      return invoke(func, FLOW_FWD(m));
    }, result_t{});
}

}

#endif




#ifndef FLOW_OP_ZIP_WITH_HPP_INCLUDED
#define FLOW_OP_ZIP_WITH_HPP_INCLUDED

namespace flow {

namespace detail {

template <typename Func, typename... Flows>
struct zip_with_adaptor : flow_base<zip_with_adaptor<Func, Flows...>> {

    using item_type = std::invoke_result_t<Func&, item_t<Flows>...>;

    constexpr explicit zip_with_adaptor(Func func, Flows&&... flows)
        : func_(std::move(func)), flows_(FLOW_FWD(flows)...)
    {}

    constexpr auto next() -> maybe<item_type>
    {
        auto maybes = std::apply([](auto&... args) {
            return std::tuple<next_t<Flows>...>{args.next()...};
        }, flows_);

        const bool all_engaged = std::apply([](auto&... args) {
            return (static_cast<bool>(args) && ...);
        }, maybes);

        if (all_engaged) {
            return {std::apply([this](auto&&... args) {
                return invoke(func_, *FLOW_FWD(args)...);
            }, std::move(maybes))};
        }
        return {};
    }

private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    std::tuple<Flows...> flows_;
};

// Specialisation for the common case of zipping two flows
template <typename Func, typename F1, typename F2>
struct zip_with_adaptor<Func, F1, F2> : flow_base<zip_with_adaptor<Func, F1, F2>>
{
    using item_type = std::invoke_result_t<Func&, item_t<F1>, item_t<F2>>;

    constexpr zip_with_adaptor(Func func, F1&& f1, F2&& f2)
        : func_(std::move(func)),
          f1_(std::move(f1)),
          f2_(std::move(f2))
    {}

    constexpr auto next() -> maybe<item_type>
    {
        auto m1 = f1_.next();
        auto m2 = f2_.next();

        if ((bool) m1 && (bool) m2) {
            return invoke(func_, *std::move(m1), *std::move(m2));
        }
        return {};
    }

private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    F1 f1_;
    F2 f2_;
};

struct zip_with_fn {
    template <typename Func, typename Flowable0, typename... Flowables>
    constexpr auto operator()(Func func, Flowable0&& flowable0, Flowables&&... flowables) const
        -> zip_with_adaptor<Func, flow_t<Flowable0>, flow_t<Flowables>...>
    {
        static_assert(is_flowable<Flowable0> && (is_flowable<Flowables> && ...),
            "All arguments to zip_with must be Flowable");
        static_assert(std::is_invocable_v<Func&, flow_item_t<Flowable0>, flow_item_t<Flowables>...>,
            "Incompatible callable passed to zip_with");

        return from(FLOW_FWD(flowable0)).zip_with(std::move(func), FLOW_FWD(flowables)...);
    }
};

}

inline constexpr auto zip_with = detail::zip_with_fn{};

template <typename D>
template <typename Func, typename... Flowables>
constexpr auto flow_base<D>::zip_with(Func func, Flowables&&... flowables) && {
    static_assert((is_flowable<Flowables> && ...),
                  "All arguments to zip_with must be Flowable");
    static_assert(std::is_invocable_v<Func&, item_t<D>, item_t<flow_t<Flowables>>...>,
                  "Incompatible callable passed to zip_with");

    return detail::zip_with_adaptor<Func, D, flow_t<Flowables>...>{
        std::move(func), consume(), flow::from(FLOW_FWD(flowables))...
    };
}

}


#endif



#ifndef FLOW_SOURCE_ANY_HPP_INCLUDED
#define FLOW_SOURCE_ANY_HPP_INCLUDED



#include <memory>

namespace flow {

template <typename T>
struct any_flow : flow_base<any_flow<T>> {
private:
    struct iface {
        virtual auto do_next() -> maybe<T> = 0;
        virtual ~iface() = default;
    };

    template <typename F>
    struct impl : iface {
        explicit impl(F flow)
            : flow_(std::move(flow))
        {}

        auto do_next() -> maybe<T> override { return flow_.next(); }
        F flow_;
    };

    std::unique_ptr<iface> ptr_;

public:
    template <typename F,
        std::enable_if_t<!std::is_same_v<remove_cvref_t<F>, any_flow>, int> = 0,
        std::enable_if_t<is_flow<F> && std::is_same_v<item_t<F>, T>, int> = 0>
    any_flow(F flow)
        : ptr_(std::make_unique<impl<F>>(std::move(flow)))
    {}

    auto next() -> maybe<T> { return ptr_->do_next(); }
};


template <typename T>
struct any_flow_ref : flow_base<any_flow_ref<T>> {
private:
    using next_fn_t = auto (void*) -> maybe<T>;

    template <typename F>
    static constexpr next_fn_t* next_fn_impl = +[](void* ptr) {
        return static_cast<F*>(ptr)->next();
    };

    void* ptr_;
    next_fn_t* next_fn_;

public:
    template <typename F,
        std::enable_if_t<!std::is_same_v<std::remove_reference_t<F>, any_flow_ref>, int> = 0,
        std::enable_if_t<is_flow<F> && std::is_same_v<item_t<F>, T>, int> = 0>
    constexpr any_flow_ref(F& flow)
        : ptr_(std::addressof(flow)),
          next_fn_(next_fn_impl<F>)
    {}

    constexpr auto next() -> maybe<T> { return next_fn_(ptr_); }
};

}

#endif


#ifndef FLOW_SOURCE_ASYNC_HPP_INCLUDED
#define FLOW_SOURCE_ASYNC_HPP_INCLUDED



#ifdef FLOW_HAVE_COROUTINES



#include <experimental/coroutine>

namespace flow {

template <typename T>
struct async : flow_base<async<T>>
{
    struct promise_type;

    using handle_type = std::experimental::coroutine_handle<promise_type>;

    struct promise_type {
    private:
        maybe<T> value{};

    public:
        auto initial_suspend() {
            return std::experimental::suspend_always{};
        };

        auto final_suspend() {
            return std::experimental::suspend_always{};
        }

        auto yield_value(std::remove_reference_t<T>& val)
        {
            value = maybe<T>(val);
            return std::experimental::suspend_always{};
        }

        auto yield_value(remove_cvref_t<T>&& val)
        {
            value = maybe<T>(std::move(val));
            return std::experimental::suspend_always{};
        }

        auto extract_value() -> maybe<T>
        {
            auto ret = std::move(value);
            value.reset();
            return ret;
        }

        auto get_return_object()
        {
            return async{handle_type::from_promise(*this)};
        }

        auto unhandled_exception()
        {
            std::terminate();
        }

        auto return_void()
        {
            return std::experimental::suspend_never{};
        }
    };

    auto next() -> maybe<T>
    {
        if (!coro.done()) {
            coro.resume();
            return coro.promise().extract_value();
        }
        return {};
    }

    async(async&& other) noexcept
        : coro(std::move(other).coro)
    {
        other.coro = nullptr;
    }

    async& operator=(async&& other) noexcept
    {
        std::swap(coro, other.coro);
    }

    ~async()
    {
        if (coro) {
            coro.destroy();
        }
    }

private:
    explicit async(handle_type&& handle)
        : coro(std::move(handle))
    {}

    handle_type coro;
};

} // namespace flow

#endif // FLOW_HAVE_COROUTINES

#endif


#ifndef FLOW_SOURCE_C_STR_HPP_INCLUDED
#define FLOW_SOURCE_C_STR_HPP_INCLUDED



namespace flow {

template <typename CharT, typename Traits = std::char_traits<CharT>>
struct c_str : flow_base<c_str<CharT, Traits>> {

    constexpr explicit c_str(CharT* str)
        : str_(str)
    {}

    constexpr auto next() -> maybe<CharT&>
    {
        CharT& c = str_[idx_];
        if (Traits::eq(c, CharT{})) {
            return {};
        }
        ++idx_;
        return {c};
    }

private:
    CharT* str_;
    dist_t idx_ = 0;
};

}

#endif



#ifndef FLOW_SOURCE_GENERATE_HPP_INCLUDED
#define FLOW_SOURCE_GENERATE_HPP_INCLUDED



namespace flow {

inline constexpr struct generate_fn {
private:
    template <typename Func>
    struct generator : flow_base<generator<Func>> {

        constexpr explicit generator(Func func)
            : func_(std::move(func))
        {}

        constexpr auto next() -> maybe<std::invoke_result_t<Func&>>
        {
            return func_();
        }

    private:
        FLOW_NO_UNIQUE_ADDRESS Func func_;
    };

public:
    template <typename Func>
    constexpr auto operator()(Func func) const
    {
        static_assert(std::is_invocable_v<Func&>);
        return generator<Func>{std::move(func)};
    }

} generate;

}

#endif


#ifndef FLOW_SOURCE_INTS_HPP_INCLUDED
#define FLOW_SOURCE_INTS_HPP_INCLUDED




namespace flow {

constexpr inline struct iota_fn {

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

constexpr inline struct ints_fn {
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


#ifndef FLOW_SOURCE_FROM_ISTREAM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_ISTREAM_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename CharT, typename Traits>
struct istream_ref {
    std::basic_istream<CharT, Traits>* ptr_;

    template <typename T>
    friend constexpr auto& operator>>(istream_ref& self, T& item)
    {
        return *self >> item;
    }
};

template <typename>
inline constexpr bool is_istream = false;

template <typename CharT, typename Traits>
inline constexpr bool is_istream<std::basic_istream<CharT, Traits>> = true;

template <typename T, typename CharT, typename Traits>
struct istream_flow : flow_base<istream_flow<T, CharT, Traits>> {
private:
    using istream_type = std::basic_istream<CharT, Traits>;
    istream_type* is_;
    T item_ = T();

public:
    explicit istream_flow(std::basic_istream<CharT, Traits>& is)
        : is_(std::addressof(is))
    {}

    // Move-only
    istream_flow(istream_flow&&) noexcept = default;
    istream_flow& operator=(istream_flow&&) noexcept = default;

    auto next() -> maybe<T>
    {
        if (!(*is_ >> item_)) { return {}; }
        return {item_};
    }
};

} // namespace detail

template <typename T, typename CharT, typename Traits>
auto from_istream(std::basic_istream<CharT, Traits>& is)
{
    return detail::istream_flow<T, CharT, Traits>(is);
}

}

#endif


#ifndef FLOW_SOURCE_ISTREAMBUF_HPP_INCLUDED
#define FLOW_SOURCE_ISTREAMBUF_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename CharT, typename Traits>
struct istreambuf_flow : flow_base<istreambuf_flow<CharT, Traits>> {

    using streambuf_type = std::basic_streambuf<CharT, Traits>;

    explicit istreambuf_flow(streambuf_type* buf)
        : buf_(buf)
    {}

    auto next() -> maybe<CharT>
    {
        if (!buf_) {
            return {};
        }

        auto c = buf_->sbumpc();

        if (c == Traits::eof()) {
            buf_ = nullptr;
            return {};
        }

        return Traits::to_char_type(c);
    }

private:
    streambuf_type* buf_ = nullptr;
};

struct from_istreambuf_fn {

    template <typename CharT, typename Traits>
    auto operator()(std::basic_streambuf<CharT, Traits>* buf) const
    {
        return istreambuf_flow<CharT, Traits>(buf);
    }

    template <typename CharT, typename Traits>
    auto operator()(std::basic_istream<CharT, Traits>& stream) const
    {
        return istreambuf_flow<CharT, Traits>(stream.rdbuf());
    }
};

} // namespace detail

inline constexpr auto from_istreambuf = detail::from_istreambuf_fn{};

} // namespace flow

#endif


#ifndef FLOW_SOURCE_OF_HPP_INCLUDED
#define FLOW_SOURCE_OF_HPP_INCLUDED



#include <array>

namespace flow {

namespace detail {

template <typename T, std::size_t N>
using array_flow = decltype(flow::from(std::declval<std::array<T, N>>()));

}

template <typename T, std::size_t N>
struct of : flow_base<of<T, N>> {
    template <typename... Args>
    constexpr explicit of(Args&&... args)
        : arr_(flow::from(std::array<T, N>{FLOW_FWD(args)...}))
    {}

    constexpr auto next() -> maybe<T&> {
        return arr_.next();
    }

private:
    detail::array_flow<T, N> arr_;
};

template <typename T, typename... U>
of(T, U...) -> of<T, 1 + sizeof...(U)>;

template <typename T>
struct empty : flow_base<empty<T>> {

    explicit constexpr empty() = default;

    constexpr auto next() -> maybe<T> { return {}; }
};

}

#endif


#endif

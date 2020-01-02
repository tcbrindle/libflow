
#ifndef FLOW_HPP_INCLUDED
#define FLOW_HPP_INCLUDED


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


#ifndef FLOW_CORE_INVOKE_HPP_INCLUDED
#define FLOW_CORE_INVOKE_HPP_INCLUDED



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


#ifndef FLOW_CORE_FLOW_BASE_HPP_INCLUDED
#define FLOW_CORE_FLOW_BASE_HPP_INCLUDED



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
    using std::optional<T>::reset;
    using std::optional<T>::operator bool;

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

    explicit constexpr operator bool() const { return ptr_ != nullptr; }

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

    explicit constexpr operator bool() const { return ptr_ != nullptr; }

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


#ifndef FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED
#define FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace flow {

template <typename T>
using remove_cvref_t = std::remove_const_t<std::remove_reference_t<T>>;

template <typename F>
using next_t = decltype(std::declval<F&>().next());

template <typename F>
using item_t = typename next_t<F>::value_type;

template <typename F>
using value_t = remove_cvref_t<item_t<F>>;

using dist_t = std::make_signed_t<std::size_t>;

template <typename T>
struct maybe;

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

template <typename I>
inline constexpr bool is_flow
    = std::is_base_of_v<flow_base<I>, I> &&
      std::is_convertible_v<I&, flow_base<I>&> &&
      detail::has_next<I>;

} // namespace flow

#endif


#include <iosfwd>  // for stream_to()
#include <string>  // for to_string()
#include <vector>  // for to_vector()

namespace flow {

template <typename Derived>
struct flow_base {
private:
    constexpr auto derived() & -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto derived() && -> Derived&& { return static_cast<Derived&&>(*this); }
    constexpr auto consume() -> Derived&& { return std::move(*this).derived(); }

protected:
    ~flow_base() = default;

public:
    template <typename D = Derived>
    constexpr auto advance(dist_t dist) -> maybe<item_t<D>>
    {
        for (dist_t i = 0; i < dist - 1; i++) {
            derived().next();
        }

        return derived().next();
    }

    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init
    {
        while (auto m = derived().next()) {
            init = invoke(func, std::move(init), std::move(m));
            if (!init) {
                break;
            }
        }
        return init;
    }

    template <typename Func>
    constexpr auto try_for_each(Func func)
    {
        using result_t = std::invoke_result_t<Func&, maybe<item_t<Derived>>&&>;
        return consume().try_fold([&func](auto&& e, auto m) {
            return invoke(func, std::move(m));
        }, result_t{});
    }

    template <typename Adaptor, typename... Args>
    constexpr auto apply(Adaptor&& adaptor, Args&&... args) && -> decltype(auto)
    {
        return invoke(FLOW_FWD(adaptor), consume(), FLOW_FWD(args)...);
    }

    // Reductions of various kinds
    template <typename Func, typename Init>
    constexpr auto fold(Func func, Init init) && -> Init;

    template <typename Func>
    constexpr auto fold(Func func) &&;

    template <typename Func>
    constexpr auto for_each(Func func) &&;

    // Consumes the flow, returning the number of items for which @pred
    // returned true
    template <typename Pred>
    constexpr auto count_if(Pred pred) && -> dist_t;

    /// Consumes the flow, returning the number of items it contained
    constexpr auto count() && -> dist_t
    {
        return consume().count_if([](auto&& /*unused*/) { return true; });
    }

    /// Consumes the flow, returning a count of the number of items which
    /// compared equal to @item, using comparator @cmp
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto count(const T& item, Cmp cmp = {}) && -> dist_t
    {
        return consume().count_if([&item, &cmp] (auto&& val) {
            return invoke(cmp, val, item);
        });
    }

    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto find(const T& item, Cmp cmp = {})
    {
        struct out {
           maybe<item_t<Derived>> val{};
           constexpr explicit operator bool() const { return !val; }
       };

       return consume().try_for_each([&item, &cmp](auto m) {
             return invoke(cmp, *m, item) ? out{std::move(m)} : out{};
       }).val;
    }

    /// Returns true if the flow contains the given item, or false otherwise
    /// This function is short-circuiting: it will stop looking for an item
    /// as soon as it finds one
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto contains(const T& item, Cmp cmp = {}) -> bool
    {
        return static_cast<bool>(consume().find(item, std::move(cmp)));
    }

    /// Consumes the flow, returning the sum of elements using `operator+`
    template <typename D = Derived>
    constexpr auto sum() && -> value_t<D>
    {
        return consume().fold(std::plus<>{});
    }

    /// Consumes the flow, returning the product of the elements using `operator*`
    template <typename D = Derived>
    constexpr auto product() && -> value_t<D>
    {
        static_assert(std::is_constructible_v<value_t<Derived>, int>,
            "Flow's value type must be constructible from a literal 1");
        return consume().fold(std::multiplies<>{}, value_t<Derived>{1});
    }

    /// Consumes the flow, returning the smallest element.
    /// If the flow is empty, return an empty `maybe`
    template <typename Cmp = std::less<>>
    constexpr auto min(Cmp cmp = Cmp{}) &&
    {
        return consume().next().map([this, &cmp] (auto&& init) {
          return consume().fold([&cmp](auto&& val, auto&& acc) -> decltype(auto) {
            return invoke(cmp, val, acc) ? FLOW_FWD(val) : FLOW_FWD(acc);
          }, FLOW_FWD(init));
        });
    }

    /// Consumes the flow, returning the largest element.
    /// If the flow is empty, return an empty `maybe`
    template <typename Cmp = std::less<>>
    constexpr auto max(Cmp cmp = Cmp{}) &&
    {
        return consume().next().map([this, &cmp] (auto&& init) {
            return consume().fold([&cmp](auto&& val, auto acc) {
                return !invoke(cmp, val, acc) ? FLOW_FWD(val) : std::move(acc);
            }, FLOW_FWD(init));
        });
    }

    /// Iterates over the flow, returning true if all the items satisfy the predicate
    /// Returns true for an empty flow
    template <typename Pred>
    constexpr auto all(Pred pred) -> bool
    {
        static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                      "Predicate must be callable with the Flow's item_type,"
                      " and must return bool");
        return consume().try_fold([&pred](bool acc, auto m) {
            return acc && invoke(pred, *std::move(m));
        }, true);
    }

    /// Iterates over the flow, returning false if any element satisfies the predicate.
    /// Returns true for an empty flow
    template <typename Pred>
    constexpr auto none(Pred pred) -> bool
    {
        static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                      "Predicate must be callable with the Flow's item_type,"
                      " and must return bool");
        return consume().all([&pred] (auto&& val) {
            return !invoke(pred, FLOW_FWD(val));
        });
    }

    /// Iterates over the flow, returning true if any element satisfies the predicate.
    /// Returns false for an empty flow
    template <typename Pred>
    constexpr auto any(Pred pred) -> bool
    {
        return !consume().none(std::move(pred));
    }

    /// Consumes the flow, returning true if the elements are sorted according to @cmp.
    template <typename Cmp = std::less<>>
    constexpr auto is_sorted(Cmp cmp = Cmp{}) && -> bool
    {
        return consume().try_fold([last = derived().next(), &cmp]
        (auto acc, auto next) mutable {
            if (invoke(cmp, *next, *last)) {
                return false;
            } else {
                last = std::move(next);
                return true;
            }
        }, true);

    }

    template <typename NextFn>
    constexpr auto adapt(NextFn next_fn) &&;

    template <typename Func>
    constexpr auto map(Func func) &&
    {
        static_assert(std::is_invocable_v<Func&, item_t<Derived>&&>,
            "Incompatible callable passed to map()");
        static_assert(!std::is_void_v<std::invoke_result_t<Func&, item_t<Derived>&&>>,
            "May cannot be used with a function returning void");
        return consume().adapt([self = consume(), func = std::move(func)] () mutable {
            return self.next().map(func);
        });
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

    template <typename = Derived>
    constexpr auto copy() && -> decltype(auto)
    {
        if constexpr (std::is_lvalue_reference_v<item_t<Derived>>) {
            return consume().template as<value_t<Derived>>();
        } else {
            return consume();
        }
    }

    template <typename = Derived>
    constexpr auto move() && -> decltype(auto)
    {
        if constexpr (std::is_lvalue_reference_v<item_t<Derived>>) {
            return consume()
                .template as<std::remove_reference_t<item_t<Derived>>&&>();
        } else {
            return consume();
        }
    }

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
                        return o;
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

    template <typename = Derived>
    constexpr auto take(dist_t count) &&
    {
        auto fn = [self = consume(), count = count] () mutable -> maybe<item_t<Derived>>
        {
            if (count > 0) {
                --count;
                return self.next();
            }
            return {};
        };

        return consume().adapt(std::move(fn));
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
        auto fn = [self = consume(), stride = stride] () mutable
        {
            auto m = self.next();
            self.advance(stride - 1);
            return m;
        };
        return consume().adapt(std::move(fn));
    }

    template <typename = Derived>
    constexpr auto cycle() &&
    {
        auto fn = [cur = derived(), initial = consume(), done = false] () mutable
            -> maybe<item_t<Derived>>
        {
            if (!done) {
                auto m = cur.next();
                if (m) {
                    return m;
                } else {
                    cur = initial;
                    auto m2 = cur.next();
                    if (!m2) {
                        done = true;
                    }
                }
            }
            return {};
        };
        return consume().adapt(std::move(fn));
    }

    template <typename Flow>
    constexpr auto chain(Flow other) &&
    {
        static_assert(std::is_same_v<item_t<Derived>, item_t<Flow>>,
            "Flows used with chain() must have the exact same item type");

        auto fn = [self = consume(),
                   other = std::move(other),
                   first = true] () mutable -> maybe<item_t<Derived>> {
          if (first) {
              if (auto m = self.next()) {
                  return m;
              }
              first = false;
          }
          return other.next();
        };
        return consume().adapt(std::move(fn));
    }

    template <typename = Derived>
    constexpr auto flatten() &&;

    template <typename Func>
    constexpr auto flat_map(Func func) &&;

    template <typename Flow>
    constexpr auto zip_with(Flow other) &&
    {
        using item_type = std::pair<item_t<Derived>, item_t<Flow>>;

        auto fn = [self = consume(), other = std::move(other)] () mutable
            -> maybe<item_type>
        {
            auto m1 = self.next();
            if (!m1) { return {}; }

            auto m2 = other.next();
            if (!m2) { return {}; }

            return item_type{*std::move(m1), *std::move(m2)};
        };
        return consume().adapt(std::move(fn));
    }

    constexpr auto enumerate() &&;

    template <std::size_t N>
    constexpr auto elements() &&
    {
        return consume().map(
            [] (auto&& val) -> decltype(std::get<N>(FLOW_FWD(val))) {
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

    template <typename = Derived>
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
        return consume().template to<std::vector>();
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

    friend auto operator<<(std::ostream& os, flow_base&& self) -> std::ostream&
    {
        std::move(self).stream_to(os);
        return os;
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
template <typename>
constexpr auto flow_base<D>::to_range() &&
{
    return detail::flow_range<D>{std::move(*this).derived()};
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


#ifndef FLOW_OP_COUNT_IF_HPP_INCLUDED
#define FLOW_OP_COUNT_IF_HPP_INCLUDED


#ifndef FLOW_OP_FOLD_HPP_INCLUDED
#define FLOW_OP_FOLD_HPP_INCLUDED



namespace flow {

namespace detail {

struct fold_op {

    template <typename Flow, typename Func, typename Init>
    constexpr auto operator()(Flow flow, Func func, Init init) const
    {
        static_assert(is_flow<Flow>,
                "First argument to flow::fold() must be a Flow");
        return std::move(flow).fold(std::move(func), std::move(init));
    }

    template <typename Flow, typename Func>
    constexpr auto operator()(Flow flow, Func func) const
    {
        static_assert(is_flow<Flow>,
                      "First argument to flow::fold() must be a Flow");
        return std::move(flow).fold(std::move(func));
    }
};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};

template <typename Derived>
template <typename Func, typename Init>
constexpr auto flow_base<Derived>::fold(Func func, Init init) && -> Init
{
    static_assert(std::is_invocable_v<Func&, item_t<Derived>, Init&&>,
                  "Incompatible callable passed to fold()");
    static_assert(std::is_assignable_v<Init&, std::invoke_result_t<Func&, item_t<Derived>, Init&&>>,
                  "Accumulator of fold() is not assignable from the result of the function");

    struct always {
        Init val;
        constexpr explicit operator bool() const { return true; }
    };

    return consume().try_fold([&func](always acc, auto m) {
        return always{func(std::move(acc).val, *std::move(m))};
    }, always{std::move(init)}).val;
}

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::fold(Func func) &&
{
    static_assert(std::is_default_constructible_v<value_t<Derived>>,
            "This flow's value type is not default constructible. "
            "Use the fold(function, initial_value) overload instead");
    return consume().fold(std::move(func), value_t<Derived>{});
}

}

#endif


namespace flow {

inline constexpr auto count_if = [](auto flow, auto pred)
{
    static_assert(is_flow<decltype(flow)>,
        "First argument to flow::count_if must be a Flow");
    return std::move(flow).count_if(std::move(pred));
};

template <typename Derived>
template <typename Pred>
constexpr auto flow_base<Derived>::count_if(Pred pred) && -> dist_t
{
    static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                  "Predicate must be callable with the Flow's item_type,"
                  " and must return bool");
    return consume().fold([&pred](dist_t count, auto&& val) {
      return count + static_cast<dist_t>(pred(FLOW_FWD(val)));
    }, dist_t{0});
}

} // namespace flow

#endif


#ifndef FLOW_OP_FLATTEN_HPP_INCLUDED
#define FLOW_OP_FLATTEN_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Base>
struct flatten_adaptor : flow_base<flatten_adaptor<Base>> {

    constexpr flatten_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    constexpr auto next() -> maybe<item_t<item_t<Base>>>
    {
        while (true) {
            if (!inner_) {
                inner_ = base_.next();
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
    maybe<item_t<Base>> inner_{};
};


}

template <typename Derived>
template <typename>
constexpr auto flow_base<Derived>::flatten() &&
{
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

inline constexpr auto for_each = [](auto flow, auto func) {
    static_assert(is_flow<decltype(flow)>,
        "First argument to flow::for_each() must be a Flow");
    return std::move(flow).for_each(std::move(func));
};

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::for_each(Func func) &&
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





#ifndef FLOW_SOURCE_ASYNC_HPP_INCLUDED
#define FLOW_SOURCE_ASYNC_HPP_INCLUDED



#ifdef FLOW_HAVE_COROUTINES



#include <experimental/coroutine>

#include <iostream>

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


#ifndef FLOW_SOURCE_FROM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_HPP_INCLUDED

#include <array>
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
using iter_value_t = remove_cvref_t<iter_reference_t<R>>;

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

template <typename I, typename S>
struct iterator_range {

    constexpr auto begin() const { return first; }
    constexpr auto end() const { return last; }

    I first;
    S last;
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
        idx_ += dist - 1;
        return next();
    }

private:
    R rng_{};
    dist_t idx_ = 0;
};

struct from_fn {
    template <typename R>
    constexpr auto operator()(R&& range) const
    {
        static_assert(
            is_stl_range<R>,
            "Argument to flow::from() does not look like an STL range");

        using rng_t =
            std::conditional_t<std::is_lvalue_reference_v<R>,
                               range_ref<std::remove_reference_t<R>>, R>;

        if constexpr (is_random_access_stl_range<R>) {
            return stl_ra_range_adaptor<rng_t>{FLOW_FWD(range)};
        } else if constexpr (is_fwd_stl_range<R>) {
            return stl_fwd_range_adaptor<rng_t>{FLOW_FWD(range)};
        } else {
            static_assert(
                !std::is_lvalue_reference_v<R>,
                "Input ranges must be passed as rvalues -- use std::move()");
            return stl_input_range_adaptor<R>{FLOW_FWD(range)};
        }
    }

    template <typename I, typename S>
    constexpr auto operator()(I iterator, S sentinel) const
    {
        return from_fn{}(iterator_range<I, S>{std::move(iterator), std::move(sentinel)});
    }
};

} // namespace detail

inline constexpr detail::from_fn from{};

} // namespace flow

#endif


#ifndef FLOW_SOURCE_GENERATE_HPP_INCLUDED
#define FLOW_SOURCE_GENERATE_HPP_INCLUDED



namespace flow {

inline constexpr struct {
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


#ifndef FLOW_SOURCE_OF_HPP_INCLUDED
#define FLOW_SOURCE_OF_HPP_INCLUDED



#include <array>

namespace flow {

namespace detail {

template <typename T, std::size_t N>
using array_flow = decltype(flow::from(std::declval<std::array<T, N>>()));

}

template <typename T, std::size_t N>
struct of : detail::array_flow<T, N> {
    template <typename... Args>
    constexpr explicit of(Args&&... args)
        : detail::array_flow<T, N>(flow::from(std::array<T, N>{FLOW_FWD(args)...}))
    {}
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

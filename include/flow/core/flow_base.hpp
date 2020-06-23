
#ifndef FLOW_CORE_FLOW_BASE_HPP_INCLUDED
#define FLOW_CORE_FLOW_BASE_HPP_INCLUDED

#include <flow/core/invoke.hpp>
#include <flow/core/maybe.hpp>
#include <flow/core/predicates.hpp>
#include <flow/core/type_traits.hpp>

#include <flow/source/from.hpp>

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
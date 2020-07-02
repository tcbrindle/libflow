
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

    template <typename D = Derived,
              typename = std::enable_if_t<std::is_copy_constructible_v<D>>>
    constexpr auto subflow() & -> D
    {
        return derived();
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
    /// fold(): this version takes a `maybe`, which fold() unwraps for
    /// convenience.
    ///
    /// Because this function is short-circuiting, the flow may be restarted
    /// after it has returned and the remaining items (if any) may be processed.
    ///
    /// This is the lowest-level reduction operation, on which all the other
    /// reductions are (eventually) based. For most user code, one of the
    /// higher-level operations is usually more convenient.
    ///
    /// @param func Callable with signature compatible with `(Init, maybe<next_t>) -> Init`
    /// @param init Initial value for the reduction. Must be contextually convertible to `bool`.
    /// @return The value accumulated at the point that the flow was exhausted or `(bool) init`
    ///         evaluated to `false`.
    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init;

    /// Short-circuiting version of for_each().
    ///
    /// Given a unary function `func`, repeatedly calls `func(next())`. If the
    /// return value of the function evaluates to `false`, it immediately exits,
    /// providing the last function return value.
    ///
    /// try_for_each() requires that the return type of `func` is "bool-ish":
    /// that it is default-constructible, move-assignable, and contextually
    /// convertible to `bool`.
    ///
    /// Note that the type of the parameter to `func` differs from plain
    /// for_each(): this version takes a #flow::maybe, which for_each() unwraps for
    /// convenience.
    ///
    /// Because this function is short-circuiting, the flow may be restarted
    /// after it has returned and the remaining items (if any) may be processed.
    ///
    /// This is a low-level operation, effectively a stateless version of
    /// try_fold(). For most user code, higher-level functions such as
    /// for_each() will be more convenient.
    ///
    /// @param func Callable with signature compatible with `(next_t<Flow>) -> T`,
    ///             where `T` is "bool-ish"
    /// @returns A copy of `func`
    template <typename Func>
    constexpr auto try_for_each(Func func);

    /// Exhausts the flow, performing a functional left fold operation.
    ///
    /// Given a callable `func` and an initial value `init`, calls
    /// `init = func(std::move(init), i)` for each item `i` in the flow. Once
    /// the flow is exhausted, it returns the value accumulated in `init`.
    ///
    /// This is the same operation as `std::accumulate`.
    ///
    /// @param func Callable with signature compatible with `(Init, item_t<Flow>) -> Init`
    /// @param init Initial value of the reduction
    /// @returns The accumulated value of type `Init`
    template <typename Func, typename Init>
    constexpr auto fold(Func func, Init init) -> Init;

    /// Exhausts the flow, performing a functional left fold operation.
    ///
    /// This is a convenience overload, equivalent to `fold(func, value_t{})`
    /// where `value_t` is the value type of the flow.
    ///
    /// @param func Callable with a signature compatible with `(value_t<Flow>, item_t<Flow>) -> value_t<Flow>`
    /// @returns The accumulated value of type `value_t<Flow>`
    template <typename Func>
    constexpr auto fold(Func func);

    /// Exhausts the flow, applying the given function to each element
    ///
    /// @param func A unary callable accepting this flow's item type
    /// @returns A copy of `func`
    template <typename Func>
    constexpr auto for_each(Func func) -> Func;

    /// Exhausts the flow, returning the number of items for which `pred`
    /// returned true
    ///
    /// Equivalent to `filter(pred).count()`.
    ///
    /// @param pred A predicate accepting the flow's item type
    /// @returns The number of items for which the `pred(item)` returned true
    template <typename Pred>
    constexpr auto count_if(Pred pred) -> dist_t;

    /// Exhausts the flow, returning the number of items it contained
    constexpr auto count() -> dist_t;

    /// Exhausts the flow, returning a count of the number of items which
    /// compared equal to `value`, using comparator `cmp`
    ///
    /// @param value Value which each item in the flow should be compared to
    /// @param cmp The comparator to be used, with a signature compatible with
    ///      `(T, item_t<Flow>) -> bool` (default is `std::equal<>`)
    /// @returns The number of items for which `cmp(value, item)` returned `true`
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
    /// @param value Value to find
    /// @param cmp Comparator to use for the find operation, with a signature
    ///            compatible with `(T, item_t<Flow>) -> bool` (default: `std::equal<>`)
    /// @returns A flow::maybe containing the first item for which `cmp(value, item)`
    ///          returned `true`, or an empty `maybe` if the flow was exhausted without
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
    /// @param value Value to find
    /// @param cmp Comparator to use for the find operation, with a signature
    ///           compatible with `(T, item_t<Flow>) -> bool` (default: `std::equal<>`)
    /// @returns `true` iff the flow contained an item for which `cmp(value, item)`
    ///          returned `true`.
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto contains(const T& value, Cmp cmp = {}) -> bool;

    /// Exhausts the flow, returning the sum of items using `operator+`
    ///
    /// Requires that the flow's value type is default-constructible.
    ///
    /// This is a convenience method, equivalent to `fold(std::plus<>{})`
    constexpr auto sum();

    /// Exhausts the flow, returning the product of the elements using `operator*`
    ///
    /// @note The flow's value type must be constructible from a literal `1`
    constexpr auto product();

    /// Exhausts the flow, returning the smallest item according `cmp`
    ///
    /// If several items are equally minimal, returns the first. If the flow
    /// is empty, returns an empty `maybe`.
    template <typename Cmp = std::less<>>
    constexpr auto min(Cmp cmp = Cmp{});

    /// Exhausts the flow, returning the largest item according to `cmp`
    ///
    /// If several items are equal maximal, returns the last. If the flow
    /// is empty, returns an empty `maybe`.
    template <typename Cmp = std::less<>>
    constexpr auto max(Cmp cmp = Cmp{});

    /// Exhausts the flow, returning both the minimum and maximum values
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

    /// Processes the flow, returning true if the elements are sorted according to
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

    /// Consumes the flow, returning a new flow which lazily invokes the given
    /// callable for each item as it is processed.
    ///
    /// Another way of thinking about it is that `map` takes a flow whose item
    /// type is `T` and turns it into a flow whose item type is `R`, where `R`
    /// is the result of applying `func` to each item.
    ///
    /// It is equivalent to C++20 `std::views::transform`.
    ///
    /// @param func A callable with signature compatible with `(item_t<F>) -> R`
    /// @return A new flow whose item type is `R`, the return type of `func`
    template <typename Func>
    constexpr auto map(Func func) &&;

    /// Consumes the flow, returning a new flow which casts each item to type `T`,
    /// using `static_cast`.
    ///
    /// Equivalent to:
    ///
    /// `map([](auto&& item) -> T { return static_cast<T>(forward(item)); })`
    ///
    /// @tparam T The type that items should be cast to. May not be `void`.
    /// @return A new flow whose item type is `T`.
    template <typename T>
    constexpr auto as() &&;

    /// Consumes the flow, returning an adaptor which dereferences each item
    /// using unary `operator*`.
    ///
    /// This is useful when you have a flow of a dereferenceable type (for
    /// example a pointer, a `flow::maybe` or a `std::optional`), and want a
    /// flow of (references to) the values they contain.
    ///
    /// Equivalent to:
    ///
    /// `map([](auto&& item) -> decltype(auto) { return *forward(item); })`
    ///
    /// @warning This function **does not** check whether the items are
    /// non-null (or equivalent) before dereferencing. See `filter_deref()` for
    /// a safer, checking version.
    ///
    /// @return An adaptor that dereferences each item of the original flow
    constexpr auto deref() &&;

    /// Consumes the flow, returning an adaptor which copies every item.
    ///
    /// If the flow's item type is an lvalue reference type `T&`, this returns
    /// a new flow whose item type is `remove_cvref<T>`, effectively copying each
    /// item as it is dereferenced.
    ///
    /// If the flow's item type is not an lvalue reference, this adaptor has
    /// no effect.
    ///
    /// @return A flow adaptor which copies each item of the original flow.
    constexpr auto copy() && -> decltype(auto);

    /// Consumes the flow, returning an adaptor which casts each item to
    /// an xvalue, allowing it to be moved from.
    ///
    /// If the flow's item type is an lvalue reference type `T&`, this returns
    /// a new flow whose item type is `T&&`, allowing the item to be moved from.
    ///
    /// If the flow's item type is not an lvalue reference, this adaptor has
    /// no effect.
    ///
    /// @return A flow adaptor which casts each item to an xvalue.
    constexpr auto move() && -> decltype(auto);

    /// Consumes the flow, returning a new flow which yields the same items
    /// but calls @func at each call to `next()`, passing it a const reference
    /// to the item.
    ///
    /// `inspect()` is intended as an aid to debugging. It can be
    /// inserted at any point in a pipeline, and allows examining the items
    /// in the flow at that point (for example, to print them).
    template <typename Func>
    constexpr auto inspect(Func func) &&;

    /// Consumes the flow, returning a new flow containing only those items for
    /// which `pred(item)` returned `true`.
    ///
    /// @param pred Predicate with signature compatible with `(const item_t<F>&) -> bool`
    /// @return A new filter adaptor.
    template <typename Pred>
    constexpr auto filter(Pred pred) &&;

    /// Consumes the flow, returning a new flow which skips the first `count` items.
    ///
    /// @param count The number of items to skip
    /// @return A new drop adaptor
    constexpr auto drop(dist_t count) &&;

    /// Consumes the flow, returning a new flow which skips items until `pred`
    /// returns `false`.
    ///
    /// One `pred` has returned false, `drop_while` has done its job and the
    /// rest of the items will be returned as normal.
    ///
    /// @param pred A predicate with signature compatible with `(const item_t<F>&) -> bool)`.
    /// @return A new drop_while adaptor.
    template <typename Pred>
    constexpr auto drop_while(Pred pred) &&;

    /// Consumes the flow, returning a new flow which yields at most `count` items.
    ///
    /// The returned flow may return fewer than `count` items if the underlying
    /// flow contained fewer items.
    ///
    /// The returned flow is sized if the original flow is itself sized or infinite.
    ///
    /// @param count Maximum number of items this flow should return.
    /// @return A new take adaptor
    constexpr auto take(dist_t count) &&;

    /// Consumes the flow, returning a new flow which takes items until the
    /// predicate returns `false`
    ///
    /// One `pred` has returned `false`, `take_while` has done its job and
    /// will not yield any more items.
    ///
    /// @param pred Callable with signature compatible with `(const value_t<Flow>&) -> bool`.
    /// @return A new take_while adaptor
    template <typename Pred>
    constexpr auto take_while(Pred pred) &&;

    /// Consume the flow, returning a new flow which starts at the same point and
    /// then advances by `step` positions at each call to `next()`.
    ///
    /// @param step The step size, must be >= 1
    /// @return A new stride adaptor
    constexpr auto stride(dist_t step) &&;

    /// Consumes the flow, returning a new flow which yields fixed-size flows
    /// of `window_size`, stepping by `step_size` ever iteration.
    ///
    /// If `partial_windows` is `false` (the default), then windows smaller than
    /// `window_size` at the end of the flow are ignored.
    ///
    /// If `window_size` is 1, then this is like `stride(step_size)`, except that
    /// the inner item type is a flow (whereas `stride()` flattens these).
    ///
    /// If `window_size` is equal to `step_size`, then this is equivalent to
    /// `chunk()`, except that the undersized chunk at the end may be omitted
    /// depending on the value of `partial_windows`.
    ///
    /// @param window_size The size of the windows to generate. Must be >= 1
    /// @param step_size The step size to use. Must be >= 1, default is 1
    /// @param partial_windows Whether windows of size less than `window_size`
    ///                        occurring at the end of the flow should be included.
    ///                        Default is `false`.
    /// @return A new slide adaptor
    constexpr auto slide(dist_t window_size,
                         dist_t step_size = 1,
                         bool partial_windows = false) &&;

    /// Consumes the flow, returning a new flow which endlessly repeats its items.
    ///
    /// @note Requires that the flow is copy-constructable and copy-assignable.
    ///
    /// \return A new cycle adaptor
    constexpr auto cycle() &&;

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
        return consume().group_by(flow::pred::eq(std::move(delimiter))).stride(2);
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
    constexpr auto equal(Flowable&& flowable) -> bool
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
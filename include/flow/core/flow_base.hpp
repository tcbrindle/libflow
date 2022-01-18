
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_CORE_FLOW_BASE_HPP_INCLUDED
#define FLOW_CORE_FLOW_BASE_HPP_INCLUDED

#include <flow/core/functional.hpp>
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

    /// Performs a left fold using `func`, passing the first element of the
    /// flow as the initial value of the accumulator.
    ///
    /// If the flow is empty, returns an empty `maybe`. Otherwise, returns a
    /// `maybe` containing the accumulated result.
    ///
    /// @param func Callable with a signature compatible with `(value_t<Flow>, item_t<Flow>) -> value_t<Flow>`
    /// @returns The accumulated value of the fold, wrapped in a `flow::maybe`
    template <typename Func>
    constexpr auto fold_first(Func func);

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
    template <typename T, typename Cmp = equal_to>
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
    template <typename T, typename Cmp = equal_to>
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
    template <typename T, typename Cmp = equal_to>
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
    template <typename Cmp = less>
    constexpr auto min(Cmp cmp = Cmp{});

    /// Exhausts the flow, returning the largest item according to `cmp`
    ///
    /// If several items are equal maximal, returns the last. If the flow
    /// is empty, returns an empty `maybe`.
    template <typename Cmp = less>
    constexpr auto max(Cmp cmp = Cmp{});

    /// Exhausts the flow, returning both the minimum and maximum values
    /// according to `cmp`.
    ///
    /// If several items are equal minimal, returns the first. If several items
    /// are equally maximal, returns the last. If the flow is empty, returns
    /// an empty `maybe`.
    template <typename Cmp = less>
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
    template <typename Cmp = less>
    constexpr auto is_sorted(Cmp cmp = Cmp{}) -> bool;

    /// Processes the flows, returning true if both flows contain equal items
    /// (according to `cmp`), and both flows end at the same time.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flows when it finds the first non-equal pair of items.
    ///
    /// @param flowable
    /// @param cmp Comparator to use, defaulting to std::equal_to<>
    /// @return True if the flows contain equal items, and false otherwise
    template <typename Flowable, typename Cmp = equal_to>
    constexpr auto equal(Flowable&& flowable, Cmp cmp = Cmp{}) -> bool;

    /// Given a reversible flow, returns a new flow which processes items
    /// from back to front.
    ///
    /// The returned adaptor effectively swaps the actions of the `next()` and
    /// `next_back()` functions.
    ///
    /// @return A new flow which runs in the opposite direction
    constexpr auto reverse() &&;

    template <typename Func,
              typename D = Derived,
              typename Init = value_t<D>>
    constexpr auto scan(Func func, Init init = Init{}) &&;

    constexpr auto partial_sum() &&;

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
    /// non-null (or equivalent) before dereferencing. See `deref()` for
    /// a safer, checking version.
    ///
    /// @return An adaptor that dereferences each item of the original flow
    constexpr auto unchecked_deref() &&;

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

    /// Consumes the flow, returning an adaptor which converts each reference
    /// item to a const reference.
    ///
    /// If the flow's item type is an lvalue reference type `T&`, this returns
    /// a new flow whose item type is `const T&`.
    ///
    /// If the flow's item type is not an lvalue reference, this adaptor has
    /// no effect.
    ///
    /// @return A flow adaptor which converts each item to a const reference
    constexpr auto as_const() && -> decltype(auto);

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

    /// Consumes the flow, returning an adaptor which dereferences each item
    /// using unary `operator*`.
    ///
    /// This is useful when you have a flow of a dereferenceable type (for
    /// example a pointer, a `flow::maybe` or a `std::optional`), and want a
    /// flow of (references to) the values they contain.
    ///
    /// Unlike `unchecked_deref()`, this adaptor first attempts to check whether
    /// item contains a value, using a static_cast to bool. If the check returns
    /// false, the item is skipped.
    ///
    /// Equivalent to:
    ///
    /// `filter([](const auto& i) { return static_cast<bool>(i); }.unchecked_deref()`
    ///
    /// @return An adaptor that dereferences each item of the original flow
    constexpr auto deref() &&;

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
    /// @return A new cycle adaptor
    constexpr auto cycle() &&;

    /// Takes a set of flows and creates a new flow which iterates over each of
    /// them, one after the other.
    ///
    /// @note All flows passed to `chain` must have the exact same item type
    ///
    /// @param flowables A list of Flowable objects to chain
    /// @return A new chain adaptor
    template <typename... Flowables>
    constexpr auto chain(Flowables&&... flowables) &&;

    /// Returns an adaptor which alternates an item from the first flow, followed
    /// by an item from the second flow, followed by the the next item from
    /// the first flow, and so on.
    ///
    /// The adaptor is exhausted when either of the two flows is exhausted.
    ///
    /// @note Both flows must have the exact same item type
    ///
    /// @param with A Flowable object with which to interleave
    /// @return A new interleave adaptor
    template <typename Flowable>
    constexpr auto interleave(Flowable&& with) &&;

    /// Returns an adaptor which flattens a nested flow of Flowable objects.
    ///
    /// This is equivalent to `std::ranges::join`
    ///
    /// @return A new flatten adaptor
    constexpr auto flatten() &&;

    /// Given a callable which returns a Flowable type, applies the function
    /// to each item in the flow and then flattens the resulting flows.
    ///
    /// Equivalent to `map(func).flatten()`.
    ///
    /// @param func A callable with signature compatible with `(item_t<Flow>) -> R`,
    ///             where `R` is a Flowable type
    /// @return A new adaptor which maps then flattens
    template <typename Func>
    constexpr auto flat_map(Func func) &&;

    /// Given a set of Flowable objects, processes them in lockstep, returning
    /// a `std::pair` or `std::tuple` of their items.
    ///
    /// The resulting adaptor will be exhausted when the first of the component
    /// flows is exhausted.
    ///
    /// @param flowables A pack of flowable objects
    /// @return A new zip adaptor
    template <typename... Flowables>
    constexpr auto zip(Flowables&&... flowables) &&;

    /// Adapts the flow so that it returns (index, item) pairs.
    ///
    /// Equivalent to flow::ints().zip(*this)
    ///
    /// @return A new enumerate adaptor
    constexpr auto enumerate() &&;

    /// Given a set of flowable objects, processes them in lockstep and
    /// calls `func` passing the flows' items as arguments. The result of
    /// `func` is used as the item type of the resulting flow.
    ///
    /// Think of this an an n-ary version of `map()`.
    ///
    /// @param func Callable with signature compatible with `(item_t<Flows>...) -> R`,
    ///             where R may not be `void`
    /// @param flowables A pack of flowable objects
    /// @return A new flow whose item type is `R`.
    template <typename Func, typename... Flowables>
    constexpr auto zip_with(Func func, Flowables&&... flowables) &&;

    /// Given a callable and a pack of multipass-flowable objects, returns a new
    /// flow which calls `func` with all combinations of items from each of the
    /// flows. This can be used as an alternative to nested `for` loops.
    ///
    /// @param func Callable with signature compatible with `(item_t<Flows>...) -> R`,
    ///             where `R` may not be `void`
    /// @param flowables A pack of flowable objects, all of which must be multipass
    /// @return A new flow whose item type is `R`.
    template <typename Func, typename... Flowables>
    constexpr auto cartesian_product_with(Func func, Flowables&&... flowables) &&;

    /// Given a set of mutipass-flowable objects, returns a new flow whose item
    /// type is a `std::pair` or `std::tuple` with successive combinations of
    /// all the items of all the flows. This can be used as an alternative to
    /// nested `for` loops.
    ///
    /// @param flowables A pack of flowable objects, all of which must be multipass
    /// @return An adapted flow whose item type is a `std::tuple` or `std::pair`.
    template <typename... Flowables>
    constexpr auto cartesian_product(Flowables&&... flowables) &&;

    /// Turns a flow into a flow-of-flows, where each inner flow ("group") is
    /// delimited by the return value of `func`.
    ///
    /// The key function `func` must return a type that is equality comparable,
    /// and must always return the same output when given the same input, no
    /// matter how many times it is called.
    ///
    /// @note This adaptor requires a multipass flow
    ///
    /// \param func Callable with signature (item_t<Flow>) -> K, where K is
    ///             equality comparable
    /// \return A new group_by adaptor
    template <typename Key>
    constexpr auto group_by(Key func) &&;

    /// Consumes the flow, returning a new flow where each item is a flow
    /// containing `size` items. The last chunk may have fewer items.
    ///
    /// @note This adaptor requires a multipass flow
    ///
    /// @param size The size of each chunk. Must be greater than zero.
    /// @return A new chunk adaptor
    constexpr auto chunk(dist_t size) &&;

    /// Splits a flow into a flow-of-flows, using `delimiter`.
    ///
    /// @note This adaptor requires a multipass flow.
    ///
    /// @param delimiter The delimiter to use for the split operation
    /// @return A new split adaptor
    template <typename D = Derived>
    constexpr auto split(value_t<D> delimiter) &&;

    /// If the flow's item type is tuple-like (e.g. `std::tuple` or `std::pair`),
    /// returns an adaptor whose item type is the `N`th element of each tuple,
    /// where indexing is zero-based.
    ///
    /// @tparam N Tuple index which the adaptor should return
    /// @return A new tuple element adaptor
    template <std::size_t N>
    constexpr auto elements() &&;

    /// If the flow's item type is a "tuple-like" key-value pair, returns an
    /// adaptor whose item type is the "key" part.
    ///
    /// This is a convenience function equivalent to `elements<0>()`.
    constexpr auto keys() &&;

    /// If the flow's item type is a "tuple-like" key-value pair, returns an
    /// adaptor whose item type is the "value" part.
    ///
    /// This is a convenience function equivalent to `elements<1>()`.
    constexpr auto values() &&;

    /// Consumes the flow, returning an object that is compatible with the
    /// standard library's range protocol (that is, it has begin() and end()
    /// methods which return iterators).
    ///
    /// It is mostly useful to allow flows to be used with built-in range-for loops,
    /// or to pass flows to standard library algorithms.
    ///
    /// @return A new range object
    constexpr auto to_range() &&;

    /// Consumes the flow, converting it into a new object of type `C`.
    ///
    /// If `std::is_constructible_v<C, Flow&&>` is true, then this is equivalent
    /// to `return C(move(flow));`.
    ///
    /// Otherwise, the flow is first converted to a range, and this function
    /// returns `C(first, last);` where `first` and `last` are the iterator and
    /// sentinel types of the range
    ///
    /// @tparam C The container type to construct, for example `std::vector<int>`.
    /// @return A new object of type `C`
    template <typename C>
    constexpr auto to() && -> C;

    /// Consumes the flow, converting it into a specialisation of template `C`.
    ///
    /// This overload uses constructor template argument deduction (CTAD) to
    /// work out the container template arguments.
    ///
    /// @tparam C A class template name, for example `std::vector`.
    /// @return A new object which is a specialisation of type `C.
    template <template <typename...> typename C>
    constexpr auto to() &&;

    /// Consumes the flow, converting it into a `std::vector`.
    ///
    /// Equivalent to `to_vector<value_t<Flow>>()`.
    ///
    /// @returns: A new `std::vector<T>`, where `T` is the value type of the flow
    auto to_vector() &&;

    /// Consumes the flow, converting it into a `std::vector<T>`.
    ///
    /// Requires that the flow's value type is convertible to `T`.
    ///
    /// Equivalent to `to<std::vector<T>>()`.
    ///
    /// @tparam T The value type of the resulting vector
    /// @return A new `std::vector<T>` containing the items of this flow.
    template <typename T>
    auto to_vector() && -> std::vector<T>;

    /// Consumes the flow, converting it into a `std::string`.
    ///
    /// Requires that the flow's value type is convertible to `char`.
    ///
    /// Equivalent to `to<std::string>()`, but fractionally less typing.
    ///
    /// @return A new `std::string`.
    auto to_string() && -> std::string;

    /// Consumes the flow, collecting its items into an object which can in turn
    /// be converted to a standard library container.
    constexpr auto collect() &&;

    /// Exhausts the flow, writing each item to the given output iterator
    ///
    /// This is equivalent to the standard library's `std::copy()`.
    ///
    /// @param oiter An output iterator which is compatible with this flow's item
    /// type
    /// @return A copy of the provided iterator
    template <typename Iter>
    constexpr auto output_to(Iter oiter) -> Iter;

    /// Exhausts the flow, writing each item to the given output stream.
    ///
    /// Each item is written to the stream using `stream << separator << item`
    /// (except for the first item, which is not preceded by a separator).
    /// The separator may be any C++ type which is "output streamable". The
    /// default separator is `", "`.
    ///
    /// @param os A specialisation of `std::basic_ostream` to write to
    /// @param sep Separator to use, defaulting to `", "`.
    /// @returns The provided `basic_ostream` reference
    template <typename Sep = const char*, typename CharT, typename Traits>
    constexpr auto write_to(std::basic_ostream<CharT, Traits>& os, Sep sep = ", ")
        -> std::basic_ostream<CharT, Traits>&;
};

}

#endif

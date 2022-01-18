
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_ZIP_WITH_HPP_INCLUDED
#define FLOW_OP_ZIP_WITH_HPP_INCLUDED

#include <flow/core/flow_base.hpp>
#include <flow/source/from.hpp>

#include <limits>

namespace flow {

namespace detail {

// A zipped flow is sized if all its component flows are sized or infinite,
// and at least one of them is non-infinite.
// Fold expressions are cool.
template <typename... Flows>
inline constexpr bool is_sized_zip =
    ((is_sized_flow<Flows> || is_infinite<Flows>) && ...) &&
    !(is_infinite<Flows> && ...);

template <typename F>
constexpr auto size_or_infinity(const F& flow) -> dist_t
{
    if constexpr (is_infinite<F>) {
        return std::numeric_limits<dist_t>::max();
    } else if (is_sized_flow<F>) {
        return flow.size();
    }
}

template <typename Func, typename... Flows>
struct zip_with_adaptor : flow_base<zip_with_adaptor<Func, Flows...>> {

    static_assert(sizeof...(Flows) > 0);

    // ICE in MSVC if this is changed to invoke_result_t *sigh*
    using item_type = typename std::invoke_result<Func&, item_t<Flows>...>::type;

    static constexpr bool is_infinite = (is_infinite_flow<Flows> && ...);

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

    template <bool B = (is_multipass_flow<Flows> && ...),
              typename = std::enable_if_t<B>>
    constexpr auto subflow() & -> zip_with_adaptor<function_ref<Func>, subflow_t<Flows>...>
    {
        return std::apply([&func_ = func_](auto&... args) {
            return zip_with_adaptor<function_ref<Func>, subflow_t<Flows>...>(func_, args.subflow()...);
        }, flows_);
    }

    template <bool B = is_sized_zip<Flows...>,
              typename = std::enable_if_t<B>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return std::apply([](auto const&... args) {
            // Clang doesn't like flow::of(...).min() here, not sure why
            auto ilist = {size_or_infinity(args)...};
            return *flow::min(ilist);
        }, flows_);
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

    static constexpr bool is_infinite = is_infinite_flow<F1> && is_infinite_flow<F2>;

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

    template <typename S1 = F1, typename S2 = F2>
    constexpr auto subflow() & -> zip_with_adaptor<function_ref<Func>, subflow_t<S1>, subflow_t<S2>>
    {
        return {func_, f1_.subflow(), f2_.subflow()};
    }

    template <bool B = is_sized_zip<F1, F2>,
              typename = std::enable_if_t<B>>
    constexpr auto size() const -> dist_t
    {
        return min(size_or_infinity(f1_), size_or_infinity(f2_));
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

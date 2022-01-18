
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_CHAIN_HPP_INCLUDED
#define FLOW_OP_CHAIN_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename... Flows>
struct chain_adaptor : flow_base<chain_adaptor<Flows...>>
{
private:
    template <typename...>
    friend struct chain_adaptor;

    std::tuple<Flows...> flows_;
    std::uint8_t idx_ = 0; // Assume we never chain more than 255 flows...

    using idx_seq = std::make_index_sequence<sizeof...(Flows)>;

    using item_type = item_t<std::tuple_element_t<0, decltype(flows_)>>;

    template <std::size_t N, std::size_t... I>
    constexpr auto get_next_impl(idx_seq) -> maybe<item_type>
    {
        if constexpr (N < sizeof...(Flows)) {
            if (N == idx_) {
                return std::get<N>(flows_).next();
            } else {
                return get_next_impl<N + 1>(idx_seq{});
            }
        } else {
            return {};
        }
    }

    constexpr auto get_next() -> maybe<item_type>
    {
        return get_next_impl<0>(idx_seq{});
    }

    template <std::size_t N, typename Func, typename Init, std::size_t... I>
    constexpr auto try_fold_impl(Func& func, Init init, idx_seq)
    {
        if constexpr (N < sizeof...(Flows)) {
            if (N == idx_) {
                init = std::get<N>(flows_).try_fold(func, std::move(init));
                if (!init) {
                    return init;
                } else {
                    ++idx_;
                }
            }
            return try_fold_impl<N+1>(func, std::move(init), idx_seq{});
        } else {
            return init;
        }
    }

public:
    static constexpr bool is_infinite = (is_infinite_flow<Flows> || ...);

    constexpr explicit chain_adaptor(Flows&&... flows)
        : flows_(std::move(flows)...)
    {}

    constexpr auto next() -> maybe<item_type>
    {
        if (idx_ < sizeof...(Flows)) {
            if (auto m = get_next()) {
                return m;
            }
            ++idx_;
            return next();
        }
        return {};
    }

    template <bool B = (flow::is_sized_flow<Flows> && ...)>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return std::apply([](auto const&... args) {
            return (args.size() + ...);
        }, flows_);
    }

    template <bool B = (flow::is_multipass_flow<Flows> && ...),
              typename = std::enable_if_t<B>>
    constexpr auto subflow() &
    {
        auto s = std::apply([](auto&... args) {
            return chain_adaptor<subflow_t<Flows>...>(args.subflow()...);
        }, flows_);
        s.idx_ = idx_;
        return s;
    }

    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init
    {
        return try_fold_impl<0>(func, std::move(init), idx_seq{});
    }
};

// Specialisation for the common case of chaining two flows
template <typename Flow1, typename Flow2>
struct chain_adaptor<Flow1, Flow2> : flow_base<chain_adaptor<Flow1, Flow2>> {

    static constexpr bool is_infinite = is_infinite_flow<Flow1> || is_infinite_flow<Flow2>;

    constexpr chain_adaptor(Flow1&& flow1, Flow2&& flow2)
        : flow1_(std::move(flow1)),
          flow2_(std::move(flow2))
    {}

    constexpr auto next() -> next_t<Flow1>
    {
        if (first_)  {
            if (auto m = flow1_.next()) {
                return m;
            }
            first_ = false;
        }
        return flow2_.next();
    }

    template <typename F1 = Flow1, typename F2 = Flow2,
              typename = std::enable_if_t<is_sized_flow<F1> && is_sized_flow<F2>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return flow1_.size() + flow2_.size();
    }

    template <typename F1 = Flow1, typename F2 = Flow2>
    constexpr auto subflow() & -> chain_adaptor<subflow_t<F1>, subflow_t<F2>>
    {
        auto s = chain_adaptor<subflow_t<F1>, subflow_t<F2>>{flow1_.subflow(), flow2_.subflow()};
        s.first_ = first_;
        return s;
    }

    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init
    {
        if (first_) {
            init = flow1_.try_fold(function_ref{func}, std::move(init));
            if (!init) {
                return init;
            }
            first_ = false;
        }
        return flow2_.try_fold(std::move(func), std::move(init));
    }

private:
    template <typename...>
    friend struct chain_adaptor;

    Flow1 flow1_;
    Flow2 flow2_;
    bool first_ = true;
};

}

inline constexpr auto chain = [](auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)> && (is_flowable<decltype(flowables)> && ...),
                  "All arguments to flow::chain() must be Flowable types");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable0))).chain(FLOW_FWD(flowables)...);
};

template <typename D>
template <typename... Flowables>
constexpr auto flow_base<D>::chain(Flowables&&... flowables) &&
{
    static_assert((is_flowable<Flowables> && ...),
                  "Arguments to chain() must be flowable types");
    static_assert((std::is_same_v<item_t<D>, item_t<flow_t<Flowables>>> && ...),
                  "All flows passed to chain() must have the exact same item type");

    return detail::chain_adaptor<D, flow_t<Flowables>...>(
        consume(), FLOW_COPY(flow::from(FLOW_FWD(flowables)))...);
}

}

#endif

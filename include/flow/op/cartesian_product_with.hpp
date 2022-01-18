
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED
#define FLOW_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Func, typename Flow0, typename... Flows>
struct cartesian_product_with_adaptor
    : flow_base<cartesian_product_with_adaptor<Func, Flow0, Flows...>>
{
private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    std::tuple<Flow0, Flows...> flows_;
    std::tuple<subflow_t<Flows>...> subs_;
    std::tuple<next_t<Flow0>, next_t<Flows>...> maybes_{};

    inline static constexpr std::size_t N = 1 + sizeof...(Flows);
    using item_type = std::invoke_result_t<Func, item_t<Flow0>&, item_t<subflow_t<Flows>>&...>;

    template <std::size_t I>
    constexpr bool advance_()
    {
        if constexpr (I == 0) {
            if (!std::get<I>(maybes_)) {
                std::get<I>(maybes_) = std::get<I>(flows_).next();

                if (!std::get<I>(maybes_)) {
                    return false; // We're done
                }

                std::get<I>(subs_) = std::get<I + 1>(flows_).subflow();
            }
        } else {
            if (!std::get<I>(maybes_)) {
                std::get<I>(maybes_) = std::get<I - 1>(subs_).next();
                if (!std::get<I>(maybes_)) {
                    std::get<I - 1>(maybes_) = {};
                    return advance_<I - 1>();
                }

                if constexpr (I < N - 1) {
                    std::get<I>(subs_) = std::get<I + 1>(flows_).subflow();
                }
            }
        }

        if constexpr (I < N - 1) {
            return advance_<I + 1>();
        } else {
            return true;
        }
    }

    template <std::size_t... I>
    constexpr auto apply_(std::index_sequence<I...>) -> decltype(auto)
    {
        return invoke(func_, *std::get<I>(maybes_)..., *std::get<N - 1>(std::move(maybes_)));
    }

public:

    static constexpr bool is_infinite =
        is_infinite_flow<Flow0> || (is_infinite_flow<Flows> || ...);

    constexpr cartesian_product_with_adaptor(Func func, Flow0&& flow0, Flows&&... flows)
        : func_(std::move(func)),
          flows_(std::move(flow0), std::move(flows)...),
          subs_(flows.subflow()...)
    {}

    constexpr auto next() -> maybe<item_type>
    {
        std::get<N - 1>(maybes_) = {};

        if (!advance_<0>()) {
            return {};
        } else {
            return {apply_(std::make_index_sequence<N - 1>{})};
        }
    }

    template <bool B = is_sized_flow<Flow0> && (is_sized_flow<Flows> && ...)>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return std::apply([](auto const&... args) { return (args.size() * ...); }, flows_);
    }
};

// Specialisation for the common case of twp flows
template <typename Func, typename Flow1, typename Flow2>
struct cartesian_product_with_adaptor<Func, Flow1, Flow2>
    : flow_base<cartesian_product_with_adaptor<Func, Flow1, Flow2>>
{
private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    Flow1 f1_;
    Flow2 f2_;
    subflow_t<Flow2> s2_ = f2_.subflow();
    next_t<Flow1> m1_{};

    using item_type = std::invoke_result_t<Func, item_t<Flow1>&, item_t<Flow2>>;

public:
    static constexpr bool is_infinite = is_infinite_flow<Flow1> || is_infinite_flow<Flow2>;

    constexpr cartesian_product_with_adaptor(Func func, Flow1&& flow1, Flow2&& flow2)
        : func_(std::move(func)),
          f1_(std::move(flow1)),
          f2_(std::move(flow2))
    {}

    constexpr auto next() -> maybe<item_type>
    {
        while (true) {
            if (!m1_) {
                m1_ = f1_.next();
                if (!m1_) {
                    return {}; // we're done
                }

                s2_ = f2_.subflow();
            }

            auto m2 = s2_.next();
            if (!m2) {
                m1_.reset();
                continue;
            }

            return {invoke(func_, *m1_, *std::move(m2))};
        }
    }

    template <bool B = is_sized_flow<Flow1> && is_sized_flow<Flow2>>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return f1_.size() * f2_.size();
    }
};

}

inline constexpr auto cartesian_product_with =
    [](auto func, auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)>,
                  "All arguments to cartesian_product_with() must be Flowable");
    return flow::from(FLOW_FWD(flowable0))
      .cartesian_product_with(std::move(func), FLOW_FWD(flowables)...);
};

template <typename Derived>
template <typename Func, typename... Flowables>
constexpr auto flow_base<Derived>::cartesian_product_with(Func func, Flowables&&... flowables) &&
{
    static_assert((is_flowable<Flowables> && ...),
                  "All arguments to cartesian_product_with() must be Flowable");
    static_assert((is_multipass_flow<flow_t<Flowables>> && ...),
                  "To use cartesian_product_with(), all flows except the "
                  "first must be multipass");
    static_assert(std::is_invocable_v<Func&, item_t<Derived>&, flow_item_t<Flowables>&...>,
                  "Incompatible callable passed to cartesian_product_with()");

    return detail::cartesian_product_with_adaptor<Func, Derived, flow_t<Flowables>...>
        (std::move(func), consume(), flow::from(FLOW_FWD(flowables))...);
}

}

#endif


#include <flow/core/flow_base.hpp>

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

    template <bool B = (is_multipass_flow<Flows> && ...),
              typename = std::enable_if_t<B>>
    constexpr auto subflow() & -> zip_with_adaptor<Func, subflow_t<Flows>...>
    {
        return std::apply([&func_ = func_](auto&... args) {
            return zip_with_adaptor<Func, subflow_t<Flows>...>(func_, args.subflow()...);
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
    constexpr auto subflow() & -> zip_with_adaptor<Func, subflow_t<S1>, subflow_t<S2>>
    {
        return {func_, f1_.subflow(), f2_.subflow()};
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

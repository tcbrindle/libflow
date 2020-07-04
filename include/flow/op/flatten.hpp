
#ifndef FLOW_OP_FLATTEN_HPP_INCLUDED
#define FLOW_OP_FLATTEN_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

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

    template <typename F = Base,
              typename = std::enable_if_t<is_multipass_flow<flow_t<item_t<F>>>>>
    constexpr auto subflow() -> flatten_adaptor<subflow_t<F>>
    {
        if (!inner_) {
            return flatten_adaptor<subflow_t<F>>{base_.subflow()};
        }
        return flatten_adaptor<subflow_t<F>>{base_.subflow(), inner_->subflow()};
    }

private:
    template <typename>
    friend struct flatten_adaptor;

    constexpr flatten_adaptor(Base&& base, flow_t<item_t<Base>>&& inner)
            : base_(std::move(base)),
              inner_(std::move(inner))
    {}

    Base base_;
    maybe<flow_t<item_t<Base>>> inner_{};
};

}

inline constexpr auto flatten = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::flatten() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).flatten();
};

template <typename Derived>
constexpr auto flow_base<Derived>::flatten() &&
{
    static_assert(is_flowable<item_t<Derived>>,
        "flatten() requires a flow whose item type is Flowable");
    return detail::flatten_adaptor<Derived>(consume());
}

}

#endif

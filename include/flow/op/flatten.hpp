
#ifndef FLOW_OP_FLATTEN_HPP_INCLUDED
#define FLOW_OP_FLATTEN_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

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
                return m;
            } else {
                inner_ = maybe<item_t<Base>>{};
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

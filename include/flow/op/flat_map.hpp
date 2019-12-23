
#ifndef FLOW_OP_FLAT_MAP_HPP_INCLUDED
#define FLOW_OP_FLAT_MAP_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

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

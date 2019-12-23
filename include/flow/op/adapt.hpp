
#ifndef FLOW_OP_ADAPT_HPP_INCLUDED
#define FLOW_OP_ADAPT_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename NextFn>
struct adaptor : flow_base<adaptor<NextFn>> {

    constexpr explicit adaptor(NextFn&& next_fn) : next_fn_(std::move(next_fn))
    {}

    constexpr auto next() { return next_fn_(); }

private:
    FLOW_NO_UNIQUE_ADDRESS NextFn next_fn_;
};

}

template <typename D>
template <typename NextFn>
constexpr auto flow_base<D>::adapt(NextFn next_fn) &&
{
    return detail::adaptor<NextFn>(std::move(next_fn));
}

}

#endif

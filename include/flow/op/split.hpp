
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_SPLIT_HPP_INCLUDED
#define FLOW_OP_SPLIT_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct split_op {
    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable, flow_value_t<Flowable> delim) const
    {
        return FLOW_COPY(flow::from(FLOW_FWD(flowable))).split(delim);
    }
};

}

inline constexpr auto split = detail::split_op{};

template <typename Derived>
template <typename D>
constexpr auto flow_base<Derived>::split(value_t<D> delim) &&
{
    // So, this works, kinda, but sucks.
    // What we do is split on the delimiter, and then skip those groups
    // which contain the delimiter
    // Doing this with a filter would be too slow, so what we do is inspect
    // the first group (to see whether it was a delim or not), possibly
    // drop that, then continue serving up every second group.
    // I mean, it's cool that we can do this, but a dedicated adaptor would
    // be much better.
    return consume().group_by(flow::pred::eq(std::move(delim)))
            .drop_while([delim](auto f) {
                assert(f.subflow().count() > 0);
                return *f.subflow().next() == delim; })
            .stride(2);
}

}

#endif

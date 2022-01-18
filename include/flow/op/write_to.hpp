
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_WRITE_TO_HPP_INCLUDED
#define FLOW_OP_WRITE_TO_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct write_to_op {
    template <typename Flowable, typename CharT, typename Traits,
              typename Sep = const char*>
    constexpr auto operator()(Flowable&& flowable, std::basic_ostream<CharT, Traits>& os,
                              Sep sep = ", ") const
        -> std::basic_ostream<CharT, Traits>&
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::write_to() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).write_to(os, sep);
    }
};

}

inline constexpr auto write_to = detail::write_to_op{};

template <typename D>
template <typename Sep, typename CharT, typename Traits>
constexpr auto flow_base<D>::write_to(std::basic_ostream<CharT, Traits>& os, Sep sep)
    -> std::basic_ostream<CharT, Traits>&
{
    consume().for_each([&os, &sep, first = true](auto&& m) mutable {
        if (first) {
            first = false;
        } else {
            os << sep;
        }
        os << FLOW_FWD(m);
    });
    return os;
}

}

#endif


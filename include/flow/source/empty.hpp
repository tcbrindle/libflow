
#ifndef FLOW_SOURCE_EMPTY_HPP_INCLUDED
#define FLOW_SOURCE_EMPTY_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

template <typename T>
struct empty : flow_base<empty<T>> {

    explicit constexpr empty() = default;

    static constexpr auto next() -> maybe<T> { return {}; }

    static constexpr auto size() -> dist_t { return 0; }
};

}

#endif

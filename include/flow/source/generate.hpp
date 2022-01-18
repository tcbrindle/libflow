
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_GENERATE_HPP_INCLUDED
#define FLOW_SOURCE_GENERATE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr struct generate_fn {
private:
    template <typename Func>
    struct generator : flow_base<generator<Func>> {

        static constexpr bool is_infinite = true;

        constexpr explicit generator(Func func)
            : func_(std::move(func))
        {}

        constexpr auto next() -> maybe<std::invoke_result_t<Func&>>
        {
            return func_();
        }

    private:
        FLOW_NO_UNIQUE_ADDRESS Func func_;
    };

public:
    template <typename Func>
    constexpr auto operator()(Func func) const
    {
        static_assert(std::is_invocable_v<Func&>);
        return generator<Func>{std::move(func)};
    }

} generate;

}

#endif

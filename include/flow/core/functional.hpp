
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_CORE_INVOKE_HPP_INCLUDED
#define FLOW_CORE_INVOKE_HPP_INCLUDED

#include <flow/core/macros.hpp>

#include <memory>       // for std::addressof
#include <type_traits>

namespace flow {

namespace detail {

struct invoke_fn {
private:
    template <typename T, typename Type, typename T1, typename... Args>
    static constexpr auto impl(Type T::* f, T1&& t1, Args&&... args) -> decltype(auto)
    {
        if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
            if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
                return (FLOW_FWD(t1).*f)(FLOW_FWD(args)...);
            } else {
                return ((*FLOW_FWD(t1)).*f)(FLOW_FWD(args)...);
            }
        } else {
            static_assert(std::is_member_object_pointer_v<decltype(f)>);
            static_assert(sizeof...(args) == 0);
            if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
                return FLOW_FWD(t1).*f;
            } else {
                return (*FLOW_FWD(t1)).*f;
            }
        }
    }

    template <typename F, typename... Args>
    static constexpr auto impl(F&& f, Args&&... args) -> decltype(auto)
    {
        return FLOW_FWD(f)(FLOW_FWD(args)...);
    }

public:
    template <typename F, typename... Args>
    constexpr auto operator()(F&& func, Args&&... args) const
        noexcept(std::is_nothrow_invocable_v<F, Args...>)
        -> std::invoke_result_t<F, Args...>
    {
        return impl(FLOW_FWD(func), FLOW_FWD(args)...);
    }
};

}

inline constexpr auto invoke = detail::invoke_fn{};

namespace detail {

// This is basically a cut-down, constexpr version of std::reference_wrapper
template <typename F>
struct function_ref {

    constexpr function_ref(F& func) : f_(std::addressof(func)) {}

    function_ref(F&&) = delete;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::invoke_result_t<F&, Args...>
    {
        return invoke(*f_, FLOW_FWD(args)...);
    }

private:
    F* f_;
};

}

struct equal_to {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(static_cast<bool>(FLOW_FWD(t) == FLOW_FWD(u)))
    {
        return static_cast<bool>(FLOW_FWD(t) == FLOW_FWD(u));
    }
};

struct not_equal_to {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(!equal_to{}(FLOW_FWD(t), FLOW_FWD(u)))
    {
        return !equal_to{}(FLOW_FWD(t), FLOW_FWD(u));
    }
};

struct less {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(static_cast<bool>(FLOW_FWD(t) < FLOW_FWD(u)))
    {
        return static_cast<bool>(FLOW_FWD(t) < FLOW_FWD(u));
    }
};

struct greater {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(less{}(FLOW_FWD(u), FLOW_FWD(t)))
    {
        return less{}(FLOW_FWD(u), FLOW_FWD(t));
    }
};

struct less_equal {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(!less{}(FLOW_FWD(u), FLOW_FWD(t)))
    {
        return !less{}(FLOW_FWD(u), FLOW_FWD(t));
    }
};

struct greater_equal {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(!less{}(FLOW_FWD(t), FLOW_FWD(u)))
    {
        return !less{}(FLOW_FWD(t), FLOW_FWD(u));
    }
};

namespace detail {

// Reimplementations of std::min and std::max so we don't need to drag in <algorithm>
inline constexpr struct min_fn {
    template <typename T>
    constexpr auto operator()(const T& lhs, const T& rhs) const -> decltype(auto)
    {
        return lhs < rhs ? lhs : rhs;
    }

} min;

inline constexpr struct max_fn {
    template <typename T>
    constexpr auto operator()(const T& lhs, const T& rhs) const -> const T&
    {
        return rhs < lhs ? lhs : rhs;
    }
} max;

}

}

#endif

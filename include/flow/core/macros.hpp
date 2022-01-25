
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_CORE_MACROS_HPP_INCLUDED
#define FLOW_CORE_MACROS_HPP_INCLUDED

#include <ciso646>

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(no_unique_address)
#define FLOW_HAS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
#endif

#ifdef FLOW_HAS_NO_UNIQUE_ADDRESS
#define FLOW_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define FLOW_NO_UNIQUE_ADDRESS
#endif

#if defined(__cpp_impl_coroutine)  // We have language support for C++20 coroutines
#  if defined(__has_include)
#    if __has_include(<coroutine>)
#      define FLOW_HAVE_COROUTINES
#      define FLOW_HAVE_CPP20_COROUTINES
#    endif  // __has_include(<coroutine>)
#  endif // __has_include
#elif defined(__cpp_coroutines) // We have language support for TS coroutines
#  if defined(__has_include)
#    if __has_include(<experimental/coroutine>)
#        define FLOW_HAVE_COROUTINES
#        define FLOW_HAVE_TS_COROUTINES
#    endif // __has_include(<experimental/coroutine>)
#  endif // __has_include
#endif

#define FLOW_FWD(x) (static_cast<decltype(x)&&>(x))

#define FLOW_COPY(x) (static_cast<flow::remove_cvref_t<decltype(x)>>(x))

#define FLOW_FOR(VAR_DECL, ...) \
    if (auto&& _flow = (__VA_ARGS__); true) \
        while (auto _a = _flow.next()) \
            if (VAR_DECL = *std::move(_a); true)

#endif

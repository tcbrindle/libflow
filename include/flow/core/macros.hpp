
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

#if defined(__cpp_coroutines)
#  if defined(__has_include) // If we can, check for <experimental/coroutine>
#    if __has_include(<experimental/coroutine>)
#      define FLOW_HAVE_COROUTINES
#    endif // __cpp_coroutines
#  else // can't check for it, let's hope for the best
#    define FLOW_HAVE_COROUTINES
#  endif // __has_include
#endif // __cpp_coroutines

#define FLOW_FWD(x) (static_cast<decltype(x)&&>(x))

#define FLOW_COPY(x) (static_cast<flow::remove_cvref_t<decltype(x)>>(x))

#define FLOW_FOR(VAR_DECL, ...) \
    if (auto&& _flow = (__VA_ARGS__); true) \
        while (auto _a = _flow.next()) \
            if (VAR_DECL = *std::move(_a); true)

#endif

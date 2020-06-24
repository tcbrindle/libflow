
#ifndef FLOW_CORE_MACROS_HPP_INCLUDED
#define FLOW_CORE_MACROS_HPP_INCLUDED

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
#define FLOW_HAVE_COROUTINES
#endif

#define FLOW_FWD(x) (static_cast<decltype(x)&&>(x))

#define FLOW_COPY(x) (static_cast<remove_cvref_t<decltype(x)>>(x))

#define FLOW_FOR(VAR_DECL, ...) \
    if (auto&& _flow = (__VA_ARGS__); true) \
        while (auto _a = _flow.next()) \
            if (VAR_DECL = *std::move(_a); true)

#endif

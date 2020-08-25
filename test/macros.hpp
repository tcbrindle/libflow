
#include <ciso646>

#if defined(__GNUC__) && !defined(__clang__)
#define COMPILER_IS_GCC 1
#else
#define COMPILER_IS_GCC 0
#endif

#if defined(__clang__)
#define COMPILER_IS_CLANG 1
#else
#define COMPILER_IS_CLANG 0
#endif

#if defined(_MSC_VER)
#define COMPILER_IS_MSVC 1
#else
#define COMPILER_IS_MSVC 0
#endif

// If we're using a recent libstdc++, we'll assume we're using a recent-enough
// GCC or Clang which can handle the constexpr tests
#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 9
#define HAVE_CONSTEXPR_TESTS 1
// Otherwise,
#elif defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 8000
#define HAVE_CONSTEXPR_TESTS 1
#endif

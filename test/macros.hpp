
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

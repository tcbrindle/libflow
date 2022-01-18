
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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

//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#pragma once

// this header must be generated by the build system (may be empty)
#include <nil/actor/detail/build_config.hpp>

// Platform-specific adjustments.
#define ACTOR_CACHE_LINE_SIZE 64

// Config pararameters defined by the build system (usually CMake):
//
// ACTOR_ENABLE_RUNTIME_CHECKS:
//   - check requirements at runtime
//
// ACTOR_LOG_LEVEL:
//   - denotes the amount of logging, ranging from error messages only (0)
//     to complete traces (4)

/// Denotes version of ACTOR in the format {MAJOR}{MINOR}{PATCH},
/// whereas each number is a two-digit decimal number without
/// leading zeros (e.g. 900 is version 0.9.0).
#define ACTOR_VERSION 1701

/// Defined to the major version number of ACTOR.
#define ACTOR_MAJOR_VERSION (ACTOR_VERSION / 10000)

/// Defined to the minor version number of ACTOR.
#define ACTOR_MINOR_VERSION ((ACTOR_VERSION / 100) % 100)

/// Defined to the patch version number of ACTOR.
#define ACTOR_PATCH_VERSION (ACTOR_VERSION % 100)

// This compiler-specific block defines:
// - ACTOR_DEPRECATED to annotate deprecated functions
// - ACTOR_PUSH_WARNINGS/ACTOR_POP_WARNINGS to surround "noisy" header includes
// - ACTOR_ANNOTATE_FALLTHROUGH to suppress warnings in switch/case statements
// - ACTOR_COMPILER_VERSION to retrieve the compiler version in ACTOR_VERSION format
// - One of the following:
//   + ACTOR_CLANG
//   + ACTOR_GCC
//   + ACTOR_MSVC

// sets ACTOR_DEPRECATED, ACTOR_ANNOTATE_FALLTHROUGH,
// ACTOR_PUSH_WARNINGS and ACTOR_POP_WARNINGS
#if defined(__clang__)
#define ACTOR_CLANG
#define ACTOR_LIKELY(x) __builtin_expect((x), 1)
#define ACTOR_UNLIKELY(x) __builtin_expect((x), 0)
#define ACTOR_DEPRECATED __attribute__((deprecated))
#define ACTOR_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#define ACTOR_PUSH_WARNINGS                                                                                              \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wall\"") _Pragma(                            \
        "clang diagnostic ignored \"-Wextra\"") _Pragma("clang diagnostic ignored \"-Wundef\"")                        \
        _Pragma("clang diagnostic ignored \"-Wshadow\"") _Pragma("clang diagnostic ignored \"-Wdeprecated\"") _Pragma( \
            "clang diagnostic ignored \"-Wextra-semi\"") _Pragma("clang diagnostic ignored \"-Wconversion\"")          \
            _Pragma("clang diagnostic ignored \"-Wcast-align\"") _Pragma("clang diagnostic ignored \"-Wfloat-equal\"") \
                _Pragma("clang diagnostic ignored \"-Wswitch-enum\"")                                                  \
                    _Pragma("clang diagnostic ignored \"-Wweak-vtables\"") _Pragma(                                    \
                        "clang diagnostic ignored \"-Wdocumentation\"")                                                \
                        _Pragma("clang diagnostic ignored \"-Wold-style-cast\"") _Pragma(                              \
                            "clang diagnostic ignored \"-Wsign-conversion\"")                                          \
                            _Pragma("clang diagnostic ignored \"-Wunused-template\"") _Pragma(                         \
                                "clang diagnostic ignored \"-Wshorten-64-to-32\"")                                     \
                                _Pragma("clang diagnostic ignored \"-Wunreachable-code\"") _Pragma(                    \
                                    "clang diagnostic ignored \"-Wdouble-promotion\"")                                 \
                                    _Pragma("clang diagnostic ignored \"-Wc++14-extensions\"") _Pragma(                \
                                        "clang diagnostic ignored \"-Wunused-parameter\"")                             \
                                        _Pragma("clang diagnostic ignored \"-Wnested-anon-types\"") _Pragma(           \
                                            "clang diagnostic ignored \"-Wreserved-id-macro\"")                        \
                                            _Pragma("clang diagnostic ignored \"-Wconstant-conversion\"") _Pragma(     \
                                                "clang diagnostic ignored \"-Wimplicit-fallthrough\"")                 \
                                                _Pragma("clang diagnostic ignored \"-Wused-but-marked-unused\"")       \
                                                    _Pragma("clang diagnostic ignored \"-Wdisabled-macro-expansion\"")
#define ACTOR_PUSH_UNUSED_LABEL_WARNING \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wunused-label\"")
#define ACTOR_PUSH_NON_VIRTUAL_DTOR_WARNING \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wnon-virtual-dtor\"")
#define ACTOR_PUSH_DEPRECATED_WARNING \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define ACTOR_POP_WARNINGS _Pragma("clang diagnostic pop")
#define ACTOR_ANNOTATE_FALLTHROUGH [[clang::fallthrough]]
#define ACTOR_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#if !__has_feature(cxx_thread_local)
#define ACTOR_NO_THREAD_LOCAL
#endif
#elif defined(__GNUC__)
#define ACTOR_GCC
#define ACTOR_LIKELY(x) __builtin_expect((x), 1)
#define ACTOR_UNLIKELY(x) __builtin_expect((x), 0)
#define ACTOR_DEPRECATED __attribute__((deprecated))
#define ACTOR_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#define ACTOR_PUSH_WARNINGS                                                                                        \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wshadow\"")                                \
        _Pragma("GCC diagnostic ignored \"-Wpragmas\"") _Pragma("GCC diagnostic ignored \"-Wpedantic\"")         \
            _Pragma("GCC diagnostic ignored \"-Wcast-qual\"") _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
                _Pragma("GCC diagnostic ignored \"-Wfloat-equal\"")                                              \
                    _Pragma("GCC diagnostic ignored \"-Wc++14-extensions\"")
#define ACTOR_PUSH_UNUSED_LABEL_WARNING \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wunused-label\"")
#define ACTOR_PUSH_NON_VIRTUAL_DTOR_WARNING \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wnon-virtual-dtor\"")
#define ACTOR_PUSH_DEPRECATED_WARNING \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define ACTOR_POP_WARNINGS _Pragma("GCC diagnostic pop")
#if __GNUC__ >= 7
#define ACTOR_ANNOTATE_FALLTHROUGH __attribute__((fallthrough))
#else
#define ACTOR_ANNOTATE_FALLTHROUGH static_cast<void>(0)
#endif
#define ACTOR_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
// disable thread_local on GCC/macOS due to heap-use-after-free bug:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67135
#ifdef __APPLE__
#define ACTOR_NO_THREAD_LOCAL
#endif
#elif defined(_MSC_VER)
#define ACTOR_MSVC
#define ACTOR_LIKELY(x) x
#define ACTOR_UNLIKELY(x) x
#define ACTOR_DEPRECATED
#define ACTOR_DEPRECATED_MSG(msg)
#define ACTOR_PUSH_WARNINGS __pragma(warning(push))
#define ACTOR_PUSH_UNUSED_LABEL_WARNING __pragma(warning(push)) __pragma(warning(disable : 4102))
#define ACTOR_PUSH_DEPRECATED_WARNING __pragma(warning(push))
#define ACTOR_PUSH_NON_VIRTUAL_DTOR_WARNING __pragma(warning(push))
#define ACTOR_POP_WARNINGS __pragma(warning(pop))
#define ACTOR_ANNOTATE_FALLTHROUGH static_cast<void>(0)
#define ACTOR_COMPILER_VERSION _MSC_FULL_VER
#pragma warning(disable : 4624)
#pragma warning(disable : 4800)
#pragma warning(disable : 4503)
#ifndef NOMINMAX
#define NOMINMAX
#endif    // NOMINMAX
#else
#define ACTOR_LIKELY(x) x
#define ACTOR_UNLIKELY(x) x
#define ACTOR_DEPRECATED
#define ACTOR_PUSH_WARNINGS
#define ACTOR_PUSH_NON_VIRTUAL_DTOR_WARNING
#define ACTOR_PUSH_DEPRECATED_WARNING
#define ACTOR_POP_WARNINGS
#define ACTOR_ANNOTATE_FALLTHROUGH static_cast<void>(0)
#endif

// This OS-specific block defines one of the following:
// - ACTOR_MACOS
// - ACTOR_LINUX
// - ACTOR_BSD
// - ACTOR_WINDOWS
// It also defines ACTOR_POSIX for POSIX-compatible systems
#if defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define ACTOR_IOS
#else
#define ACTOR_MACOS
#if defined(ACTOR_GCC) && !defined(_GLIBCXX_HAS_GTHREADS)
#define _GLIBCXX_HAS_GTHREADS
#endif
#endif
#elif defined(__ANDROID__)
#define ACTOR_ANDROID
#elif defined(__linux__)
#define ACTOR_LINUX
#include <linux/version.h>
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 16)
#define ACTOR_POLL_IMPL
#endif
#elif defined(__FreeBSD__)
#define ACTOR_BSD
#elif defined(__CYGWIN__)
#define ACTOR_CYGWIN
#elif defined(WIN32) || defined(_WIN32)
#define ACTOR_WINDOWS
#else
#error Platform and/or compiler not supported
#endif
#if defined(ACTOR_MACOS) || defined(ACTOR_LINUX) || defined(ACTOR_BSD) || defined(ACTOR_CYGWIN)
#define ACTOR_POSIX
#endif

#if defined(ACTOR_WINDOWS) && defined(ACTOR_CLANG)
// Fix for issue with static_cast<> in objbase.h.
// See: https://github.com/philsquared/Catch/issues/690.
struct IUnknown;
#endif

#include <cstdio>
#include <cstdlib>

// Optionally enable ACTOR_ASSERT
#ifndef ACTOR_ENABLE_RUNTIME_CHECKS
#define ACTOR_ASSERT(unused) static_cast<void>(0)
#elif defined(ACTOR_WINDOWS) || defined(ACTOR_BSD)
#define ACTOR_ASSERT(stmt)                                                       \
    if (static_cast<bool>(stmt) == false) {                                    \
        printf("%s:%u: requirement failed '%s'\n", __FILE__, __LINE__, #stmt); \
        ::abort();                                                             \
    }                                                                          \
    static_cast<void>(0)
#else    // defined(ACTOR_LINUX) || defined(ACTOR_MACOS)
#include <execinfo.h>
#define ACTOR_ASSERT(stmt)                                                       \
    if (static_cast<bool>(stmt) == false) {                                    \
        printf("%s:%u: requirement failed '%s'\n", __FILE__, __LINE__, #stmt); \
        void *array[20];                                                       \
        auto mtl_bt_size = ::backtrace(array, 20);                             \
        ::backtrace_symbols_fd(array, mtl_bt_size, 2);                         \
        ::abort();                                                             \
    }                                                                          \
    static_cast<void>(0)
#endif

// Convenience macros.
#define ACTOR_IGNORE_UNUSED(x) static_cast<void>(x)

#define ACTOR_CRITICAL(error)                                                                  \
    do {                                                                                     \
        fprintf(stderr, "[FATAL] %s:%u: critical error: '%s'\n", __FILE__, __LINE__, error); \
        ::abort();                                                                           \
    } while (false)

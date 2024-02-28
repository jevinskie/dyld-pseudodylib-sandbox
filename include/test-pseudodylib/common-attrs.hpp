#pragma once

#ifndef TEST_PSEUDODYLIB_HONOR_INLINE
#define TEST_PSEUDODYLIB_HONOR_INLINE 1
#endif

#define TEST_PSEUDODYLIB_EXPORT __attribute__((visibility("default")))
#if TEST_PSEUDODYLIB_HONOR_INLINE
#define TEST_PSEUDODYLIB_INLINE __attribute__((always_inline))
#else
#define TEST_PSEUDODYLIB_INLINE
#endif
#define TEST_PSEUDODYLIB_NOINLINE __attribute__((noinline))
#define TEST_PSEUDODYLIB_LIKELY(cond) __builtin_expect((cond), 1)
#define TEST_PSEUDODYLIB_UNLIKELY(cond) __builtin_expect((cond), 0)
#define TEST_PSEUDODYLIB_BREAK() __builtin_debugtrap()
#define TEST_PSEUDODYLIB_ALIGNED(n) __attribute__((aligned(n)))
#define TEST_PSEUDODYLIB_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), n)
#define TEST_PSEUDODYLIB_UNREACHABLE() __builtin_unreachable()

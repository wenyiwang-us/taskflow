#include <cstdio>
#include <cstdarg>
#pragma once

// ============================================================================
// C++ Versions
// ============================================================================
#define TF_CPP98 199711L
#define TF_CPP11 201103L
#define TF_CPP14 201402L
#define TF_CPP17 201703L
#define TF_CPP20 202002L

// ============================================================================
// inline and no-inline
// ============================================================================

#if defined(_MSC_VER)
  #define TF_FORCE_INLINE __forceinline
#elif defined(__GNUC__) && __GNUC__ > 3
  #define TF_FORCE_INLINE __attribute__((__always_inline__)) inline
#else
  #define TF_FORCE_INLINE inline
#endif

#if defined(_MSC_VER)
  #define TF_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) && __GNUC__ > 3
  #define TF_NO_INLINE __attribute__((__noinline__))
#else
  #define TF_NO_INLINE
#endif

// ============================================================================
// likely and unlikely
// ============================================================================

#if defined(__GNUC__)
  #define TF_LIKELY(x) (__builtin_expect((x), 1))
  #define TF_UNLIKELY(x) (__builtin_expect((x), 0))
#else
  #define TF_LIKELY(x) (x)
  #define TF_UNLIKELY(x) (x)
#endif



// ----------------------------------------------------------------------------    

#if defined(TF_USE_XQUEUE)
// ============================================================================
// debug printf with function name prefix
// ============================================================================


// void tf_debug(int kind, int level, size_t wid, const char* func, const char *msg, ...) {
//     va_list list;
//         va_start(list, msg);
//         char tabs[64], prefix[128], buf[1024];

//         // build tabs
//         int i = 0;
//         while (i < level) {
//             tabs[2 * i] = ' ';
//             tabs[2 * i + 1] = ' ';
//             i++;
//         }
//         tabs[2 * i] = '\0';

//         sprintf(prefix, "[wid=%ld]%s(%s): ", wid, tabs, func);
//         sprintf(buf, "%s%s\n", prefix, msg);

//         // print all at once so that the output is not interleaved
//         vfprintf(stderr, buf, list);
//         va_end(list);
// }


// #define TF_DEBUG(wid, msg, ...) \
//     tf_debug(0, 0, wid, __func__, msg, ##__VA_ARGS__)

#define TF_DEBUG(wid, msg, ...)

#ifdef TF_DEBUG
#undef TF_DEBUG
// empty macro for TF_DEBUG
#define TF_DEBUG(wid, msg, ...)
#endif


#endif









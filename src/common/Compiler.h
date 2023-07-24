#pragma once

#ifdef __GNUC__
#define force_inline inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define force_inline __forceinline
#else
#define force_inline inline
#endif

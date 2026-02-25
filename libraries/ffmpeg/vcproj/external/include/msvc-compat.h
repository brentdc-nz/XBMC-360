#ifndef __MSVC_COMPAT_H__
#define __MSVC_COMPAT_H__

#ifdef _MSC_VER

#define inline __inline
#define __asm__ __asm

/* C99 restrict keyword - MSVC C mode does not support it */
#define restrict
#define av_restrict

/* C99 stdint types should come from stdint.h on Xbox 360 XDK */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.718281828
#endif

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#ifndef M_LN10
#define M_LN10 2.30258509299404568402
#endif

#ifndef M_LOG2_10
#define M_LOG2_10 3.32192809488736234787
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

#ifndef NAN
static __inline float __msvc_nan(void) { unsigned int _nan_val = 0x7FC00000; return *(float*)&_nan_val; }
#define NAN __msvc_nan()
#endif

#ifndef INFINITY
static __inline float __msvc_inf(void) { unsigned int _inf_val = 0x7F800000; return *(float*)&_inf_val; }
#define INFINITY __msvc_inf()
#endif

#include <float.h>
#include <stdlib.h>
#include <string.h>

#define open _open
#define snprintf _snprintf
//#define vsnprintf _vsnprintf
#define tempnam _tempnam
#define isnan _isnan

/* strtoll is C99 - map to MSVC equivalent */
#define strtoll _strtoi64
#define strtoull _strtoui64
#define atoll _atoi64

/*
 * Math function replacements (lrint, lrintf, round, roundf, trunc, etc.)
 * are handled by libavutil/libm.h using HAVE_* guards from config.h.
 * Do NOT define them here to avoid duplicate-body errors (C2084).
 */

/* Suppress MSVC warnings common in FFmpeg code */
#pragma warning(disable:4018) /* signed/unsigned mismatch */
#pragma warning(disable:4244) /* conversion from 'type1' to 'type2', possible loss of data */
#pragma warning(disable:4305) /* truncation from 'type1' to 'type2' */
#pragma warning(disable:4996) /* deprecated POSIX names */

#endif /* _MSC_VER */

#endif /* __MSVC_COMPAT_H__ */

/*
 * jconfigint.h
 *
 * Hand-crafted internal configuration for Xbox 360 XDK (PowerPC, big-endian).
 * libjpeg-turbo 3.1.4
 *
 * Generated from jconfigint.h.in with appropriate values for the platform.
 */

/* libjpeg-turbo build number */
#define BUILD  "0"

/* How to hide global symbols. */
#define HIDDEN

/* Compiler's inline keyword */
/* #undef inline */

/* How to obtain function inlining. */
#define INLINE  __forceinline

/* How to obtain thread-local storage */
#define THREAD_LOCAL  __declspec(thread)

/* Define to the full name of this package. */
#define PACKAGE_NAME  "libjpeg-turbo"

/* Version number of package */
#define VERSION  "3.1.4"

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T  4

/* No __builtin_ctzl on MSVC */
/* #undef HAVE_BUILTIN_CTZL */

/* Xbox 360 XDK does not have intrin.h (x86-only) */
/* #undef HAVE_INTRIN_H */

/* _BitScanForward is x86-only, not available on PowerPC */
/* #undef HAVE_BITSCANFORWARD */

/* FALLTHROUGH annotation (MSVC XDK lacks __has_attribute) */
#define FALLTHROUGH

/* Xbox 360 has no environment variables / getenv_s */
#define NO_GETENV

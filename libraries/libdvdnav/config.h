/* config.h for Xbox 360 (Xenon, big-endian PowerPC) port.
   Based on the original msvc config.h. */

#ifndef LIBDVDNAV_CONFIG_H
#define LIBDVDNAV_CONFIG_H

/* #undef HAVE_DLFCN_H */ /* no dlopen on 360 - dvdcss not supported */
/* #undef HAVE_DVDCSS_DVDCSS_H */
/* #undef HAVE_INTTYPES_H */
#define HAVE_MEMORY_H 1
/* #undef HAVE_STDINT_H */
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
/* #undef HAVE_UNISTD_H */
#define PACKAGE "libdvdnav"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_NAME ""
#define PACKAGE_STRING ""
#define PACKAGE_TARNAME ""
#define PACKAGE_VERSION ""
#define STDC_HEADERS 1
#define VERSION "1.2.6"

/* Xenon (PowerPC) is big-endian: B2N_* macros become no-ops */
#define WORDS_BIGENDIAN 1

/* #undef __DARWIN__ */
/* #undef const */
#define inline __inline
/* #undef size_t */

#ifndef _SSIZE_T_DEFINED
#ifndef ssize_t
typedef __int64 ssize_t;
#endif
#define _SSIZE_T_DEFINED
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

#define snprintf _snprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp

#define S_ISDIR(m) ((m) & _S_IFDIR)
#define S_ISREG(m) ((m) & _S_IFREG)
#define S_ISBLK(m) 0
#define S_ISCHR(m) 0

#ifndef _MSC_STDINT_H_
/* use the same definitions as the bundled msc stdint.h when available */
typedef unsigned __int8     uint8_t;
typedef __int8              int8_t;
typedef unsigned __int16    uint16_t;
typedef __int16             int16_t;
typedef unsigned __int32    uint32_t;
typedef __int32             int32_t;
typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
typedef unsigned int        uintptr_t;
#endif

#endif /* LIBDVDNAV_CONFIG_H */

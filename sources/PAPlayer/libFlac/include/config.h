/* libFLAC config.h - Xbox 360 (PPC, big-endian) static library build
 * Generated for XBMC Xbox 360 port
 */

#ifndef FLAC__CONFIG_H
#define FLAC__CONFIG_H

/* Xbox 360 is PowerPC big-endian */
#define WORDS_BIGENDIAN 1

/* Target CPU */
#define FLAC__CPU_PPC 1

/* Disable all assembly - Xenon PPC is not AltiVec-compatible in the same way */
#define FLAC__NO_ASM 1

/* Static library build - FLAC_API is hardcoded empty in export.h */

/* No OGG support */
#define FLAC__HAS_OGG 0

/* Package version */
#define PACKAGE_VERSION "1.2.1"
#define VERSION "1.2.1"

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the `lround' function. */
#define HAVE_LROUND 1

/* Define for inlining - disabled to guarantee external symbol emission
   for cross-TU calls (stream_decoder.c -> bitreader.c) */
#define FLaC__INLINE

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* size_t is defined on Xbox 360 */

/* fseeko/ftello mapping for MSVC on Xbox 360 */
#define fseeko fseek
#define ftello ftell

/* Suppress winsock.h include on Xbox 360 - not needed since we're big-endian
 * and ntohl() is a no-op. We define FLAC__XBOX360 to guard the includes. */
#define FLAC__XBOX360 1

#endif /* FLAC__CONFIG_H */

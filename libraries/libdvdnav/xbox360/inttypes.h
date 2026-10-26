/* Minimal inttypes.h for the Xbox 360 (XDK CRT) build of libdvdnav.
   Integer types come from the project config.h. */
#ifndef LIBDVDNAV_X360_INTTYPES_H
#define LIBDVDNAV_X360_INTTYPES_H

#include "config.h"

#define PRId64 "I64d"
#define PRIi64 "I64i"
#define PRIu64 "I64u"
#define PRIx64 "I64x"
#define PRIX64 "I64X"
#define PRId32 "d"
#define PRIu32 "u"
#define PRIx32 "x"

#endif /* LIBDVDNAV_X360_INTTYPES_H */
/*
 * Xbox 360 compatibility stubs for missing POSIX/C functions
 * These provide definitions for symbols referenced by other FFmpeg libraries
 * (e.g., libavfilter) that cannot be rebuilt due to pre-existing issues.
 */

#include <string.h>
#include "libavutil/random_seed.h"

/* POSIX strcasecmp - map to MSVC _stricmp */
int strcasecmp(const char *s1, const char *s2)
{
    return _stricmp(s1, s2);
}

/* Old FFmpeg internal name for av_get_random_seed */
unsigned int ff_random_get_seed(void)
{
    return av_get_random_seed();
}

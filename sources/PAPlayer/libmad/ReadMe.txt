========================================================================
    STATIC LIBRARY : libmad - Xbox 360 Port
========================================================================

libmad 0.15.1b - MPEG audio decoder library.
Ported from the XBMC4Xbox codebase to Xbox 360 (PowerPC) as a static
library. Uses FPM_64BIT fixed-point math and WORDS_BIGENDIAN for PPC
big-endian. Wrapped by CLibMad (include/libmad.h) for direct static
linkage - no DLL loading.

libmad.vcxproj
    Xbox 360 static library project. Builds all libmad .c sources with
    HAVE_CONFIG_H, FPM_64BIT, and WORDS_BIGENDIAN defined.

include/config.h   - Xbox 360 build configuration
include/mad.h      - Combined public API header
include/libmad.h   - CLibMad C++ wrapper class

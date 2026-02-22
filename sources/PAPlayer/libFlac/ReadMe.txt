========================================================================
    libFlac - Xbox 360 Static Library
========================================================================

libFLAC 1.2.1 (Xiph.org) ported to Xbox 360 (PPC, big-endian).
Builds as a static library (.lib) for use in the XBMC 360 port.

Source:   libFLAC 1.2.1 by Josh Coalson / Xiph.Org Foundation
License:  BSD (see FLAC source distribution for full text)

Build Configurations:
  Debug, Release, Release_LTCG, Profile, Profile_FastCap, CodeAnalysis

Key Defines:
  HAVE_CONFIG_H, FLAC__NO_ASM, FLAC__CPU_PPC,
  FLAC__XBOX360, FLAC__HAS_OGG=0

Xbox 360 Changes (from stock libFLAC 1.2.1):
  - x86 inline asm disabled (guarded with !FLAC__XBOX360)
  - Big-endian byte order (WORDS_BIGENDIAN=1, no ntohl needed)
  - winsock.h include skipped on Xbox 360
  - utime/chmod/chown stubbed out in metadata_iterators.c
  - Explicit enum casts added for stricter PPC MSVC compiler
  - OGG support disabled (would require libogg port)

Folder Structure:
  /                 17 C source files (modified for Xbox 360)
  include/config.h  Build configuration header
  include/FLAC/     Public API headers
  include/private/  Internal implementation headers
  include/protected/ Protected internal headers
  include/share/    Shared utility headers (alloc.h)

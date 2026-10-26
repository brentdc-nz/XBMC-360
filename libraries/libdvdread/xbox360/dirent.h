/* Minimal dirent.h for the Xbox 360 (XDK CRT) build of libdvdread.
   The WIN32 code paths in dvd_reader.c do not iterate directories
   (the opendir emulation was only used by the xbmc patch), so this
   only satisfies the include. */
#ifndef LIBDVD_X360_DIRENT_H
#define LIBDVD_X360_DIRENT_H

#include <direct.h>

#endif /* LIBDVD_X360_DIRENT_H */

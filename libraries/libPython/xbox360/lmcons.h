/*
 * lmcons.h stub for Xbox 360
 *
 * Provides minimal LAN Manager constants that Python's posixmodule.c needs.
 */

#ifndef _XBOX360_LMCONS_H_SHIM
#define _XBOX360_LMCONS_H_SHIM

/* Maximum username length - used by posixmodule.c for getlogin() */
#ifndef UNLEN
#define UNLEN  256
#endif

#endif /* _XBOX360_LMCONS_H_SHIM */

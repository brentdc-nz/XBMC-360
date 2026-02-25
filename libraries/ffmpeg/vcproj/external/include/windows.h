/* windows.h redirect for Xbox 360 XDK */
/* Xbox 360 uses xtl.h instead of windows.h */
#ifndef _WINDOWS_H_XBOX360_REDIRECT
#define _WINDOWS_H_XBOX360_REDIRECT

#ifdef _XBOX
#include <xtl.h>
#else
#error "This windows.h stub is only for Xbox 360 builds"
#endif

#endif /* _WINDOWS_H_XBOX360_REDIRECT */

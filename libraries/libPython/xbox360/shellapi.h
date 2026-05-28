/*
 * shellapi.h stub for Xbox 360
 *
 * Xbox 360 does not have Shell API. Provide minimal stubs.
 */

#ifndef _XBOX360_SHELLAPI_H_SHIM
#define _XBOX360_SHELLAPI_H_SHIM

/* Nothing needed - posixmodule.c conditionally uses ShellExecute
   which we don't support on Xbox 360 */

#endif /* _XBOX360_SHELLAPI_H_SHIM */

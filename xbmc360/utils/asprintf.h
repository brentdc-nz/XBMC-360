
#ifndef _ASPRINTF_H_
#define _ASPRINTF_H_

#include <stdarg.h>

/**
 * Sets `char **' pointer to be a buffer
 * large enough to hold the formatted string
 * accepting a `va_list' args of variadic
 * arguments.
 */

int
vasprintf (char **, const char *, va_list);

/**
 * Sets `char **' pointer to be a buffer
 * large enough to hold the formatted
 * string accepting `n' arguments of
 * variadic arguments.
 */

int
asprintf (char **, const char *, ...);

#endif // ! _ASPRINTF_H_

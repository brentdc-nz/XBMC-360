/* alloc - Allocator wrappers with overflow protection
 * Copyright (C) 2007  Josh Coalson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FLAC__SHARE__ALLOC_H
#define FLAC__SHARE__ALLOC_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* WATCHOUT: for c++ you may have to #define __STDC_LIMIT_MACROS 1 real early
 * before #including this file, otherwise SIZE_MAX might not be defined
 */

#include <limits.h> /* for SIZE_MAX */
#if !defined _MSC_VER && !defined __BORLANDC__
#include <stdint.h> /* for SIZE_MAX in case limits.h didn't get it */
#endif
#include <stdlib.h> /* for size_t, malloc(), etc. */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* avoid malloc()ing 0 bytes, see:
 * https://www.cert.org/secure-coding/advisories/MEM04-C.html
 */
static __inline void *safe_malloc_(size_t size)
{
	/* malloc(0) is undefined, so always allocate at least 1 byte */
	if(size == 0)
		size++;
	return malloc(size);
}

static __inline void *safe_calloc_(size_t nmemb, size_t size)
{
	if(!nmemb || !size)
		return malloc(1);
	return calloc(nmemb, size);
}

/*@@@@ there's probably a better way to prevent overflows */
static __inline void *safe_malloc_mul_2op_(size_t size1, size_t size2)
{
	if(!size1 || !size2)
		return malloc(1);
	if(size1 > SIZE_MAX / size2)
		return 0;
	return malloc(size1*size2);
}

/* size1*size2 + size3 */
static __inline void *safe_malloc_muladd2_(size_t size1, size_t size2, size_t size3)
{
	if(!size1 || !size2)
		return safe_malloc_(size3);
	if(size1 > SIZE_MAX / size2)
		return 0;
	return safe_malloc_(size1*size2 + size3);
}

static __inline void *safe_realloc_add_2op_(void *ptr, size_t size1, size_t size2)
{
	size2 += size1;
	if(size2 < size1)
		return 0;
	return realloc(ptr, size2);
}

static __inline void *safe_realloc_mul_2op_(void *ptr, size_t size1, size_t size2)
{
	if(!size1 || !size2)
		return realloc(ptr, 0);
	if(size1 > SIZE_MAX / size2)
		return 0;
	return realloc(ptr, size1*size2);
}

/* size1 + size2 */
static __inline void *safe_malloc_add_2op_(size_t size1, size_t size2)
{
	size2 += size1;
	if(size2 < size1)
		return 0;
	return malloc(size2);
}

/* size1 + size2 + size3 */
static __inline void *safe_malloc_add_3op_(size_t size1, size_t size2, size_t size3)
{
	size2 += size1;
	if(size2 < size1)
		return 0;
	size3 += size2;
	if(size3 < size2)
		return 0;
	return malloc(size3);
}

/* size1 + size2 + size3 + size4 */
static __inline void *safe_malloc_add_4op_(size_t size1, size_t size2, size_t size3, size_t size4)
{
	size2 += size1;
	if(size2 < size1)
		return 0;
	size3 += size2;
	if(size3 < size2)
		return 0;
	size4 += size3;
	if(size4 < size3)
		return 0;
	return malloc(size4);
}

#endif

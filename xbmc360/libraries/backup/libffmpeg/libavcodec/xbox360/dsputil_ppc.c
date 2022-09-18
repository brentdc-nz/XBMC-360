/*
 * Copyright (c) 2002 Brian Foley
 * Copyright (c) 2002 Dieter Shirley
 * Copyright (c) 2003-2004 Romain Dolbeau <romain@dolbeau.org>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavcodec/dsputil.h"
#include "dsputil_ppc.h"

#include <xtl.h>

/* ***** WARNING ***** WARNING ***** WARNING ***** */
/*
clear_blocks_dcbz32_ppc will not work properly on PowerPC processors with a
cache line size not equal to 32 bytes.
Fortunately all processor used by Apple up to at least the 7450 (aka second
generation G4) use 32 bytes cache line.
This is due to the use of the 'dcbz' instruction. It simply clear to zero a
single cache line, so you need to know the cache line size to use it !
It's absurd, but it's fast...

update 24/06/2003 : Apple released yesterday the G5, with a PPC970. cache line
size: 128 bytes. Oups.
The semantic of dcbz was changed, it always clear 32 bytes. so the function
below will work, but will be slow. So I fixed check_dcbz_effect to use dcbzl,
which is defined to clear a cache line (as dcbz before). So we still can
distinguish, and use dcbz (32 bytes) or dcbzl (one cache line) as required.

see <http://developer.apple.com/technotes/tn/tn2087.html>
and <http://developer.apple.com/technotes/tn/tn2086.html>
*/
static void clear_blocks_dcbz32_ppc(DCTELEM *blocks)
{
    register int misal = ((unsigned long)blocks & 0x00000010);
    register int i = 0;
#if 1
    if (misal) {
        ((unsigned long*)blocks)[0] = 0L;
        ((unsigned long*)blocks)[1] = 0L;
        ((unsigned long*)blocks)[2] = 0L;
        ((unsigned long*)blocks)[3] = 0L;
        i += 16;
    }
    for ( ; i < sizeof(DCTELEM)*6*64-31 ; i += 32) {
        //__asm__ volatile("dcbz %0,%1" : : "b" (blocks), "r" (i) : "memory");
		__dcbz( i,blocks );
    }
    if (misal) {
        ((unsigned long*)blocks)[188] = 0L;
        ((unsigned long*)blocks)[189] = 0L;
        ((unsigned long*)blocks)[190] = 0L;
        ((unsigned long*)blocks)[191] = 0L;
        i += 16;
    }
#else
    memset(blocks, 0, sizeof(DCTELEM)*6*64);
#endif
POWERPC_PERF_STOP_COUNT(powerpc_clear_blocks_dcbz32, 1);
}

/* same as above, when dcbzl clear a whole 128B cache line
   i.e. the PPC970 aka G5 */
static void clear_blocks_dcbz128_ppc(DCTELEM *blocks)
{
    register int misal = ((unsigned long)blocks & 0x0000007f);
    register int i = 0;
#if 1
    if (misal) {
        // we could probably also optimize this case,
        // but there's not much point as the machines
        // aren't available yet (2003-06-26)
        //memset(blocks, 0, sizeof(DCTELEM)*6*64);
		XMemSet(blocks, 0, sizeof(DCTELEM)*6*64);
    }
    else
        for ( ; i < sizeof(DCTELEM)*6*64 ; i += 128) {
            //__asm__ volatile("dcbzl %0,%1" : : "b" (blocks), "r" (i) : "memory");
			__dcbz128(i,blocks);
        }
#else
    XMemSet(blocks, 0, sizeof(DCTELEM)*6*64);
#endif
POWERPC_PERF_STOP_COUNT(powerpc_clear_blocks_dcbz128, 1);
}


/* check dcbz report how many bytes are set to 0 by dcbz */
/* update 24/06/2003 : replace dcbz by dcbzl to get
   the intended effect (Apple "fixed" dcbz)
   unfortunately this cannot be used unless the assembler
   knows about dcbzl ... */
static long check_dcbzl_effect(void)
{
    register char *fakedata = av_malloc(1024);
    register char *fakedata_middle;
    register long zero = 0;
    register long i = 0;
    long count = 0;

    if (!fakedata) {
        return 0L;
    }

    fakedata_middle = (fakedata + 512);
	
    XMemSet(fakedata, 0xFF, 1024);

    /* below the constraint "b" seems to mean "Address base register"
       in gcc-3.3 / RS/6000 speaks. seems to avoid using r0, so.... */
    //__asm__ volatile("dcbzl %0, %1" : : "b" (fakedata_middle), "r" (zero));
	__dcbz128(i,fakedata_middle);

    for (i = 0; i < 1024 ; i ++) {
        if (fakedata[i] == (char)0)
            count++;
    }

    av_free(fakedata);

    return count;
}

static void prefetch_ppc(void *mem, int stride, int h)
{
    register const uint8_t *p = mem;
    do {
        //__asm__ volatile ("dcbt 0,%0" : : "r" (p));
		__dcbt(0,p);
        p+= stride;
    } while(--h);
}

static void clear_block_vmx(DCTELEM *block) {
/*
    LOAD_ZERO;
    vec_st(zero_s16v,   0, block);
    vec_st(zero_s16v,  16, block);
    vec_st(zero_s16v,  32, block);
    vec_st(zero_s16v,  48, block);
    vec_st(zero_s16v,  64, block);
    vec_st(zero_s16v,  80, block);
    vec_st(zero_s16v,  96, block);
    vec_st(zero_s16v, 112, block);
*/
	__stvx(__vzero(),	block,	0);
	__stvx(__vzero(),	block,	16);
	__stvx(__vzero(),	block,	32);
	__stvx(__vzero(),	block,	48);
	__stvx(__vzero(),	block,	64);
	__stvx(__vzero(),	block,	80);
	__stvx(__vzero(),	block,	96);
	__stvx(__vzero(),	block,	112);
}

#if 0
static int pix_norm1_altivec(uint8_t *pix, int line_size)
{
    int i;
    int s;
    __vector zero = __vzero();
/*
    vector unsigned char *tv;
    vector unsigned char pixv;
    vector unsigned int sv;
    vector signed int sum;	
*/

	__vector *tv;
    __vector pixv;
    __vector sv;
    __vector sum;

    sv = __vzero();

    s = 0;
    for (i = 0; i < 16; i++) {
        /* Read in the potentially unaligned pixels */
        //tv = (vector unsigned char *) pix;
		tv = (__vector*) pix;
        //pixv = vec_perm(tv[0], tv[1], vec_lvsl(0, pix));
		pixv = __vperm(tv[0], tv[1], __lvsl(pix,0));

        /* Square the values, and add them to our sum */
        sv = vec_msum(pixv, pixv, sv);

        pix += line_size;
    }
    /* Sum up the four partial sums, and put the result into s */
    sum = vec_sums((vector signed int) sv, (vector signed int) zero);
    sum = vec_splat(sum, 3);
    
	vec_ste(sum, 0, &s);

    return s;
}

#endif

void float_init_vmx(DSPContext* c, AVCodecContext *avctx);
void vc1dsp_init_vmx(DSPContext* dsp, AVCodecContext *avctx);
void dsputil_init_ppc(DSPContext* c, AVCodecContext *avctx)
{
    // Common optimizations whether AltiVec is available or not
    c->prefetch = prefetch_ppc;
    switch (check_dcbzl_effect()) {
        case 32:
            c->clear_blocks = clear_blocks_dcbz32_ppc;
            break;
        case 128:
            c->clear_blocks = clear_blocks_dcbz128_ppc;
            break;
        default:
            break;
    }
	
	c->clear_block = clear_block_vmx;
	//vc1dsp_init_vmx(c,avctx);
	//float_init_vmx(c,avctx);
}

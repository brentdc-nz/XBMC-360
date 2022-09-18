/*
 * GMC (Global Motion Compensation)
 * AltiVec-enabled
 * Copyright (c) 2003 Romain Dolbeau <romain@dolbeau.org>
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
//#include "util_altivec.h"
//#include "types_altivec.h"
//#include "dsputil_altivec.h"

#include <xtl.h>

/*
	Used only for encoding ?:s
  altivec-enhanced gmc1. ATM this code assume stride is a multiple of 8,
  to preserve proper dst alignment.
*/
#define GMC1_PERF_COND (h==8)
void gmc1_altivec(uint8_t *dst /* align 8 */, uint8_t *src /* align1 */, int stride, int h, int x16, int y16, int rounder)
{
    const DECLARE_ALIGNED(16, unsigned short, rounder_a) = rounder;
    const DECLARE_ALIGNED(16, unsigned short, ABCD)[8] =
        {
            (16-x16)*(16-y16), /* A */
            (   x16)*(16-y16), /* B */
            (16-x16)*(   y16), /* C */
            (   x16)*(   y16), /* D */
            0, 0, 0, 0         /* padding */
        };
	int i;
    unsigned long dst_odd = (unsigned long)dst & 0x0000000F;
    unsigned long src_really_odd = (unsigned long)src & 0x0000000F;
	
	
    //register vector unsigned char dstv, dstv2, src_0, src_1, srcvA, srcvB, srcvC, srcvD;
    //register vector unsigned short Av, Bv, Cv, Dv, rounderV, tempA, tempB, tempC, tempD;
	__vector4 dstv, dstv2, src_0, src_1, srcvA, srcvB, srcvC, srcvD;//uchar
	__vector4 Av, Bv, Cv, Dv, rounderV, tempA, tempB, tempC, tempD;//ushort
	//register const vector unsigned char vczero = (const vector unsigned char)vec_splat_u8(0);
	__vector4 vczero = __vspltisb(0);
    //register const vector unsigned short vcsr8 = (const vector unsigned short)vec_splat_u16(8);
	__vector4 vcsr8 = __vspltish(0);
    

    tempA = __lvx((unsigned short*)ABCD,0);
	//vec_splat
    Av = __vsplth(tempA, 0);
    Bv = __vsplth(tempA, 1);
    Cv = __vsplth(tempA, 2);
    Dv = __vsplth(tempA, 3);

    rounderV = __vsplth(__lvehx(&rounder_a, 0), 0);

    // we'll be able to pick-up our 9 char elements
    // at src from those 32 bytes
    // we load the first batch here, as inside the loop
    // we can re-use 'src+stride' from one iteration
    // as the 'src' of the next.
    src_0 = __lvx(src,0);
    src_1 = __lvx(src,16);
    srcvA = __vperm(src_0, src_1, __lvsl(src,0));

    if (src_really_odd != 0x0000000F) {
        // if src & 0xF == 0xF, then (src+1) is properly aligned
        // on the second vector.
        srcvB = __vperm(src_0, src_1, __lvsl(src,1));
    } else {
        srcvB = src_1;
    }
    srcvA = __vmrghb(vczero, srcvA);
    srcvB = __vmrghb(vczero, srcvB);

    for(i=0; i<h; i++) {
        dst_odd = (unsigned long)dst & 0x0000000F;
        src_really_odd = (((unsigned long)src) + stride) & 0x0000000F;

        dstv = __lvx(dst, 0);

        // we we'll be able to pick-up our 9 char elements
        // at src + stride from those 32 bytes
        // then reuse the resulting 2 vectors srvcC and srcvD
        // as the next srcvA and srcvB
        src_0 = __lvx(src, stride + 0);
        src_1 = __lvx(src, stride + 16);
        srcvC = __vperm(src_0, src_1, __lvsl(src,stride + 0));

        if (src_really_odd != 0x0000000F) {
            // if src & 0xF == 0xF, then (src+1) is properly aligned
            // on the second vector.
            srcvD = __vperm(src_0, src_1, __lvsl(src, stride + 1));
        } else {
            srcvD = src_1;
        }

        srcvC = __vmrghb(vczero, srcvC);
        srcvD = __vmrghb(vczero, srcvD);


        // OK, now we (finally) do the math :-)
        // those four instructions replaces 32 int muls & 32 int adds.
        // isn't AltiVec nice ?
        tempA = __vmladduhm (srcvA, Av, rounderV);
        tempB = __vmladduhm (srcvB, Bv, tempA);
        tempC = __vmladduhm (srcvC, Cv, tempB);
        tempD = __vmladduhm (srcvD, Dv, tempC);

        srcvA = srcvC;
        srcvB = srcvD;

        tempD = __vsrh(tempD, vcsr8);

        dstv2 = __vpkuhum(tempD, vczero);

        if (dst_odd) {
            //dstv2 = __perm(dstv, dstv2, vcprm(0,1,s0,s1));
			__vector4 ttt = {0,1,s0,s1};
			dstv2 = __vperm(dstv, dstv2, ttt);
        } else {
            //dstv2 = __perm(dstv, dstv2, vcprm(s0,s1,2,3));
			__vector4 ttt = {s0,s1,2,3};
			dstv2 = __vperm(dstv, dstv2, ttt);
        }

        __stvx(dstv2, dst, 0);

        dst += stride;
        src += stride;
    }
}

/*
 * Copyright (c) 2004 Romain Dolbeau <romain@dolbeau.org>
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
#include <xtl.h>

//#define DEBUG_ALIGNMENT
#ifdef DEBUG_ALIGNMENT
#define ASSERT_ALIGNED(ptr) assert(((unsigned long)ptr&0x0000000F));
#else
#define ASSERT_ALIGNED(ptr) ;
#endif

#define zero_u8v   __vzero() 
#define zero_s8v   __vzero() 
#define zero_u16v  __vzero() 
#define zero_s16v  __vzero() 
#define zero_u32v  __vzero() 
#define zero_s32v  __vzero() 

/* this code assume that stride % 16 == 0 */

inline __vector4 __vmladduhm ( __vector4 a, __vector4 b, __vector4 c)
{
    
}

#define CHROMA_MC8_ALTIVEC_CORE(BIAS1, BIAS2) \
        vsrc2ssH = __vmrghb(zero_u8v,vsrc2uc);\
        vsrc3ssH = __vmrghb(zero_u8v,vsrc3uc);\
\
        psum = __vmladduhm(vA, vsrc0ssH, BIAS1);\
        psum = __vmladduhm(vB, vsrc1ssH, psum);\
        psum = __vmladduhm(vC, vsrc2ssH, psum);\
        psum = __vmladduhm(vD, vsrc3ssH, psum);\
        psum = BIAS2(psum);\
        psum = __vsrh(psum, v6us);\
\
        vdst = __lvx(dst, 0);\
        ppsum = __vpkuhum(psum, psum);\
        vfdst = __vperm(vdst, ppsum, fperm);\
\
        OP_U8_ALTIVEC(fsum, vfdst, vdst);\
\
        __stvx(fsum, dst, 0);\
\
        vsrc0ssH = vsrc2ssH;\
        vsrc1ssH = vsrc3ssH;\
\
        dst += stride;\
        src += stride;

#define CHROMA_MC8_ALTIVEC_CORE_SIMPLE \
\
        vsrc0ssH = __vmrghb(zero_u8v,vsrc0uc);\
        vsrc1ssH = __vmrghb(zero_u8v,vsrc1uc);\
\
        psum = __vmladduhm(vA, vsrc0ssH, v32ss);\
        psum = __vmladduhm(vE, vsrc1ssH, psum);\
        psum = __vsrh(psum, v6us);\
\
        vdst = __lvx(dst, 0);\
        ppsum = __vpkuhum(psum, psum);\
        vfdst = __vperm(vdst, ppsum, fperm);\
\
        OP_U8_ALTIVEC(fsum, vfdst, vdst);\
\
         __stvx(fsum, dst, 0);\
\
        dst += stride;\
        src += stride;

#define noop(a) a
#define add28(a) vec_add(v28ss, a)

static void PREFIX_h264_chroma_mc8_altivec(uint8_t * dst, uint8_t * src,
                                    int stride, int h, int x, int y) {
    DECLARE_ALIGNED(16, signed int, ABCD)[4] =
                        {((8 - x) * (8 - y)),
                         ((    x) * (8 - y)),
                         ((8 - x) * (    y)),
                         ((    x) * (    y))};
    register int i;
    __vector4 fperm;
    const __vector4 vABCD = __lvx(ABCD,0);
    const __vector4 vA = __vsplth(vABCD, 1);
    const __vector4 vB = __vsplth(vABCD, 3);
    const __vector4 vC = __vsplth(vABCD, 5);
    const __vector4 vD = __vsplth(vABCD, 7);
    
	//LOAD_ZERO;
	__vector4 zerov  = __vzero();
    
	const __vector4 v32ss = __vslh(__vspltish(1),__vspltish(5));
    const __vector4 v6us = __vspltish(6);
    register int loadSecond = (((unsigned long)src) % 16) <= 7 ? 0 : 1;
    register int reallyBadAlign = (((unsigned long)src) % 16) == 15 ? 1 : 0;

    __vector4 vsrcAuc, av_uninit(vsrcBuc), vsrcperm0, vsrcperm1;
    __vector4 vsrc0uc, vsrc1uc;
    __vector4 vsrc0ssH, vsrc1ssH;
    __vector4 vsrcCuc, vsrc2uc, vsrc3uc;
    __vector4 vsrc2ssH, vsrc3ssH, psum;
    __vector4 vdst, ppsum, vfdst, fsum;
	
    if (((unsigned long)dst) % 16 == 0) {
        fperm = {
			0x10, 0x11, 0x12, 0x13,
			0x14, 0x15, 0x16, 0x17,
            0x08, 0x09, 0x0A, 0x0B,
            0x0C, 0x0D, 0x0E, 0x0F
		};
    } 
	else {
        fperm = {
			0x00, 0x01, 0x02, 0x03,
			0x04, 0x05, 0x06, 0x07,
			0x18, 0x19, 0x1A, 0x1B,
			0x1C, 0x1D, 0x1E, 0x1F
		};
	}

    vsrcAuc = __lvx(src,0);

    if (loadSecond)
        vsrcBuc = __lvx(src,16);
    vsrcperm0 = __lvsl(src,0);
    vsrcperm1 = __lvsl(src,1);

    vsrc0uc = __vperm(vsrcAuc, vsrcBuc, vsrcperm0);
    if (reallyBadAlign)
        vsrc1uc = vsrcBuc;
    else
        vsrc1uc = __vperm(vsrcAuc, vsrcBuc, vsrcperm1);

    vsrc0ssH = __vmrghb(zero_u8v,vsrc0uc);
    vsrc1ssH = __vmrghb(zero_u8v,vsrc1uc);

    if (ABCD[3]) {
        if (!loadSecond) {// -> !reallyBadAlign
            for (i = 0 ; i < h ; i++) {
                vsrcCuc = __lvx(src,stride + 0);
                vsrc2uc = __vperm(vsrcCuc, vsrcCuc, vsrcperm0);
                vsrc3uc = __vperm(vsrcCuc, vsrcCuc, vsrcperm1);

                CHROMA_MC8_ALTIVEC_CORE(v32ss, noop)
            }
        } else {
			__vector4 vsrcDuc;
            for (i = 0 ; i < h ; i++) {
                vsrcCuc = __lvx(src,stride + 0);
                vsrcDuc = __lvx(src, stride + 16);
                vsrc2uc = __vperm(vsrcCuc, vsrcDuc, vsrcperm0);
                if (reallyBadAlign)
                    vsrc3uc = vsrcDuc;
                else
                    vsrc3uc = __vperm(vsrcCuc, vsrcDuc, vsrcperm1);

                CHROMA_MC8_ALTIVEC_CORE(v32ss, noop)
            }
        }
    } else {
        __vector4 vE = __vmladduhm(vB, vC);
        if (ABCD[2]) { // x == 0 B == 0
            if (!loadSecond) {// -> !reallyBadAlign
                for (i = 0 ; i < h ; i++) {
                    vsrcCuc = __lvx(src, stride + 0);
                    vsrc1uc = __vperm(vsrcCuc, vsrcCuc, vsrcperm0);
                    CHROMA_MC8_ALTIVEC_CORE_SIMPLE

                    vsrc0uc = vsrc1uc;
                }
            } else {
                __vector4 vsrcDuc;
                for (i = 0 ; i < h ; i++) {
                    vsrcCuc = __lvx(src, stride + 0);
                    vsrcDuc = __lvx(src, stride + 15);
                    vsrc1uc = __vperm(vsrcCuc, vsrcDuc, vsrcperm0);
                    CHROMA_MC8_ALTIVEC_CORE_SIMPLE

                    vsrc0uc = vsrc1uc;
                }
            }
        } else { // y == 0 C == 0
            if (!loadSecond) {// -> !reallyBadAlign
                for (i = 0 ; i < h ; i++) {
                    vsrcCuc = __lvx(src, 0);
                    vsrc0uc = __vperm(vsrcCuc, vsrcCuc, vsrcperm0);
                    vsrc1uc = __vperm(vsrcCuc, vsrcCuc, vsrcperm1);

                    CHROMA_MC8_ALTIVEC_CORE_SIMPLE
                }
            } else {
                __vector4 vsrcDuc;
                for (i = 0 ; i < h ; i++) {
                    vsrcCuc = __lvx(src,0);
                    vsrcDuc = __lvx(src, 15);
                    vsrc0uc = __vperm(vsrcCuc, vsrcDuc, vsrcperm0);
                    if (reallyBadAlign)
                        vsrc1uc = vsrcDuc;
                    else
                        vsrc1uc = __vperm(vsrcCuc, vsrcDuc, vsrcperm1);

                    CHROMA_MC8_ALTIVEC_CORE_SIMPLE
                }
            }
        }
    }
}

#undef noop
#undef add28
#undef CHROMA_MC8_ALTIVEC_CORE

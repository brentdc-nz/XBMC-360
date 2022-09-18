/*
 * VC-1 and WMV3 decoder - DSP functions AltiVec-optimized
 * Copyright (c) 2006 Konstantin Shishkov
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
#include <xtl.h>
//#include "util_altivec.h"
//#include "dsputil_altivec.h"

/**
*	Simple Gcc to vc 
**/	
#define vec_ld(offset,addr)					__lvx(addr,offset)
#define vec_st(v,o,a)						__stvx(v,a,o)
#define vec_ste_s32(v,o,a)					__stvewx(v,a,o)

#define vec_sl_s32(a,b)						__vslw(a,b)	
#define vec_lvsl(a,b)						__lvsl(b,a);

#define vec_perm(a,b,m)						__vperm(a,b,m)

#define vec_adds_s16(a,b)					__vaddshs(a,b)

#define vec_mergeh_s16(ra,rb)				__vmrghh(ra,rb)
#define vec_mergel_s16(va,vb)				__vmrglh(va,vb)
#define vec_mergeh_u8(a,b)					__vmrghb(a,b)
#define vec_mergel_u8(a,b)					__vmrglb(a,b)

#define vec_pack_s32(a,b)					__vpkuwum(a,b)
#define vec_packsu(a,b)						__vpkshus(a,b)

#define vec_unpackl_s16(v)					__vupklsh(v)
#define vec_unpackh_s16(v)					__vupkhsh(v)

#define vec_splat_u8(i)						__vspltisb(i)
#define vec_splat_s32(i)					__vspltisw(i)
#define vec_splat_u32(i)					__vspltisw(i)

#define vec_add_s32(a,b)					__vadduwm(a,b)
#define vec_sub_s32(a,b)					__vsubuwm(a,b)
#define vec_sra_s32(a,b)					__vsraw(a,b)

// Transpose 8x8 matrix of 16-bit elements (in-place)
#define TRANSPOSE8(a,b,c,d,e,f,g,h) \
do { \
	__vector4 A1, B1, C1, D1, E1, F1, G1, H1; \
	__vector4 A2, B2, C2, D2, E2, F2, G2, H2; \
 \
    A1 = vec_mergeh_s16 (a, e); \
    B1 = vec_mergel_s16 (a, e); \
    C1 = vec_mergeh_s16 (b, f); \
    D1 = vec_mergel_s16 (b, f); \
    E1 = vec_mergeh_s16 (c, g); \
    F1 = vec_mergel_s16 (c, g); \
    G1 = vec_mergeh_s16 (d, h); \
    H1 = vec_mergel_s16 (d, h); \
 \
    A2 = vec_mergeh_s16 (A1, E1); \
    B2 = vec_mergel_s16 (A1, E1); \
    C2 = vec_mergeh_s16 (B1, F1); \
    D2 = vec_mergel_s16 (B1, F1); \
    E2 = vec_mergeh_s16 (C1, G1); \
    F2 = vec_mergel_s16 (C1, G1); \
    G2 = vec_mergeh_s16 (D1, H1); \
    H2 = vec_mergel_s16 (D1, H1); \
 \
    a = vec_mergeh_s16 (A2, E2); \
    b = vec_mergel_s16 (A2, E2); \
    c = vec_mergeh_s16 (B2, F2); \
    d = vec_mergel_s16 (B2, F2); \
    e = vec_mergeh_s16 (C2, G2); \
    f = vec_mergel_s16 (C2, G2); \
    g = vec_mergeh_s16 (D2, H2); \
    h = vec_mergel_s16 (D2, H2); \
} while (0)

// main steps of 8x8 transform
#define STEP8(s0, s1, s2, s3, s4, s5, s6, s7, vec_rnd) \
do { \
    t0 = vec_sl_s32(vec_add_s32(s0, s4), vec_2); \
    t0 = vec_add_s32(vec_sl_s32(t0, vec_1), t0); \
    t0 = vec_add_s32(t0, vec_rnd); \
    t1 = vec_sl_s32(vec_sub_s32(s0, s4), vec_2); \
    t1 = vec_add_s32(vec_sl_s32(t1, vec_1), t1); \
    t1 = vec_add_s32(t1, vec_rnd); \
    t2 = vec_add_s32(vec_sl_s32(s6, vec_2), vec_sl_s32(s6, vec_1)); \
    t2 = vec_add_s32(t2, vec_sl_s32(s2, vec_4)); \
    t3 = vec_add_s32(vec_sl_s32(s2, vec_2), vec_sl_s32(s2, vec_1)); \
    t3 = vec_sub_s32(t3, vec_sl_s32(s6, vec_4)); \
    t4 = vec_add_s32(t0, t2); \
    t5 = vec_add_s32(t1, t3); \
    t6 = vec_sub_s32(t1, t3); \
    t7 = vec_sub_s32(t0, t2); \
\
    t0 = vec_sl_s32(vec_add_s32(s1, s3), vec_4); \
    t0 = vec_add_s32(t0, vec_sl_s32(s5, vec_3)); \
    t0 = vec_add_s32(t0, vec_sl_s32(s7, vec_2)); \
    t0 = vec_add_s32(t0, vec_sub_s32(s5, s3)); \
\
    t1 = vec_sl_s32(vec_sub_s32(s1, s5), vec_4); \
    t1 = vec_sub_s32(t1, vec_sl_s32(s7, vec_3)); \
    t1 = vec_sub_s32(t1, vec_sl_s32(s3, vec_2)); \
    t1 = vec_sub_s32(t1, vec_add_s32(s1, s7)); \
\
    t2 = vec_sl_s32(vec_sub_s32(s7, s3), vec_4); \
    t2 = vec_add_s32(t2, vec_sl_s32(s1, vec_3)); \
    t2 = vec_add_s32(t2, vec_sl_s32(s5, vec_2)); \
    t2 = vec_add_s32(t2, vec_sub_s32(s1, s7)); \
\
    t3 = vec_sl_s32(vec_sub_s32(s5, s7), vec_4); \
    t3 = vec_sub_s32(t3, vec_sl_s32(s3, vec_3)); \
    t3 = vec_add_s32(t3, vec_sl_s32(s1, vec_2)); \
    t3 = vec_sub_s32(t3, vec_add_s32(s3, s5)); \
\
    s0 = vec_add_s32(t4, t0); \
    s1 = vec_add_s32(t5, t1); \
    s2 = vec_add_s32(t6, t2); \
    s3 = vec_add_s32(t7, t3); \
    s4 = vec_sub_s32(t7, t3); \
    s5 = vec_sub_s32(t6, t2); \
    s6 = vec_sub_s32(t5, t1); \
    s7 = vec_sub_s32(t4, t0); \
}while(0)

#define SHIFT_HOR8(s0, s1, s2, s3, s4, s5, s6, s7) \
do { \
    s0 = vec_sra_s32(s0, vec_3); \
    s1 = vec_sra_s32(s1, vec_3); \
    s2 = vec_sra_s32(s2, vec_3); \
    s3 = vec_sra_s32(s3, vec_3); \
    s4 = vec_sra_s32(s4, vec_3); \
    s5 = vec_sra_s32(s5, vec_3); \
    s6 = vec_sra_s32(s6, vec_3); \
    s7 = vec_sra_s32(s7, vec_3); \
}while(0)

#define SHIFT_VERT8(s0, s1, s2, s3, s4, s5, s6, s7) \
do { \
    s0 = vec_sra_s32(s0, vec_7); \
    s1 = vec_sra_s32(s1, vec_7); \
    s2 = vec_sra_s32(s2, vec_7); \
    s3 = vec_sra_s32(s3, vec_7); \
    s4 = vec_sra_s32(vec_add_s32(s4, vec_1s), vec_7); \
    s5 = vec_sra_s32(vec_add_s32(s5, vec_1s), vec_7); \
    s6 = vec_sra_s32(vec_add_s32(s6, vec_1s), vec_7); \
    s7 = vec_sra_s32(vec_add_s32(s7, vec_1s), vec_7); \
}while(0)

/* main steps of 4x4 transform */
#define STEP4(s0, s1, s2, s3, vec_rnd) \
do { \
    t1 = vec_add_s32(vec_sl_s32(s0, vec_4), s0); \
    t1 = vec_add_s32(t1, vec_rnd); \
    t2 = vec_add_s32(vec_sl_s32(s2, vec_4), s2); \
    t0 = vec_add_s32(t1, t2); \
    t1 = vec_sub_s32(t1, t2); \
    t3 = vec_sl_s32(vec_sub_s32(s3, s1), vec_1); \
    t3 = vec_add_s32(t3, vec_sl_s32(t3, vec_2)); \
    t2 = vec_add_s32(t3, vec_sl_s32(s1, vec_5)); \
    t3 = vec_add_s32(t3, vec_sl_s32(s3, vec_3)); \
    t3 = vec_add_s32(t3, vec_sl_s32(s3, vec_2)); \
    s0 = vec_add_s32(t0, t2); \
    s1 = vec_sub_s32(t1, t3); \
    s2 = vec_add_s32(t1, t3); \
    s3 = vec_sub_s32(t0, t2); \
}while (0)

#define SHIFT_HOR4(s0, s1, s2, s3) \
    s0 = vec_sra_s32(s0, vec_3); \
    s1 = vec_sra_s32(s1, vec_3); \
    s2 = vec_sra_s32(s2, vec_3); \
    s3 = vec_sra_s32(s3, vec_3);

#define SHIFT_VERT4(s0, s1, s2, s3) \
    s0 = vec_sra_s32(s0, vec_7); \
    s1 = vec_sra_s32(s1, vec_7); \
    s2 = vec_sra_s32(s2, vec_7); \
    s3 = vec_sra_s32(s3, vec_7);



/** 
*	Do inverse transform on 8x8 block
*/
static void vc1_inv_trans_8x8_vmx(DCTELEM block[64])
{
/*
    vector signed short src0, src1, src2, src3, src4, src5, src6, src7;
    vector signed int s0, s1, s2, s3, s4, s5, s6, s7;
    vector signed int s8, s9, sA, sB, sC, sD, sE, sF;
    vector signed int t0, t1, t2, t3, t4, t5, t6, t7;
*/	
	__vector4 src0, src1, src2, src3, src4, src5, src6, src7;//signed short 
    __vector4 s0, s1, s2, s3, s4, s5, s6, s7;		//signed int 
    __vector4 s8, s9, sA, sB, sC, sD, sE, sF;		//signed int 
    __vector4 t0, t1, t2, t3, t4, t5, t6, t7;		//signed int 
	/*
    const __vector4 vec_64 = vec_sl(vec_splat_s32(4), vec_splat_u32(4));//signed int 
    const __vector4 uvec_7 = vec_splat_u32(7);		//unsigned int 
    const __vector4 vec_4 = vec_splat_u32(4);		//unsigned int 
    const __vector4 vec_4s = vec_splat_s32(4);		//signed int 
    const __vector4 vec_3 = vec_splat_u32(3);		//unsigned int 
    const __vector4 vec_2 = vec_splat_u32(2);		//unsigned int 
    const __vector4 vec_1s = vec_splat_s32(1);		//signed int 
    const __vector4 vec_1 = vec_splat_u32(1);		//unsigned int 
	*/
	
	 __vector4 vec_64; //signed int 
     __vector4 vec_7; //unsigned int 
     __vector4 vec_4; 	//unsigned int 
     __vector4 vec_4s; //signed int 
     __vector4 vec_3; 	//unsigned int 
     __vector4 vec_2; 	//unsigned int 
     __vector4 vec_1s; //signed int 
     __vector4 vec_1; 	//unsigned int 
	
	
	vec_64 = vec_sl_s32(vec_splat_s32(4), vec_splat_u32(4));//signed int 
    vec_7 = vec_splat_u32(7);		//unsigned int 
    vec_4 = vec_splat_u32(4);		//unsigned int 
    vec_4s = vec_splat_s32(4);		//signed int 
    vec_3 = vec_splat_u32(3);		//unsigned int 
    vec_2 = vec_splat_u32(2);		//unsigned int 
    vec_1s = vec_splat_s32(1);		//signed int 
    vec_1 = vec_splat_u32(1);		//unsigned int 

    src0 = vec_ld(  0, block);
    src1 = vec_ld( 16, block);
    src2 = vec_ld( 32, block);
    src3 = vec_ld( 48, block);
    src4 = vec_ld( 64, block);
    src5 = vec_ld( 80, block);
    src6 = vec_ld( 96, block);
    src7 = vec_ld(112, block);

    TRANSPOSE8(src0, src1, src2, src3, src4, src5, src6, src7);
    s0 = vec_unpackl_s16(src0);
    s1 = vec_unpackl_s16(src1);
    s2 = vec_unpackl_s16(src2);
    s3 = vec_unpackl_s16(src3);
    s4 = vec_unpackl_s16(src4);
    s5 = vec_unpackl_s16(src5);
    s6 = vec_unpackl_s16(src6);
    s7 = vec_unpackl_s16(src7);
    s8 = vec_unpackh_s16(src0);
    s9 = vec_unpackh_s16(src1);
    sA = vec_unpackh_s16(src2);
    sB = vec_unpackh_s16(src3);
    sC = vec_unpackh_s16(src4);
    sD = vec_unpackh_s16(src5);
    sE = vec_unpackh_s16(src6);
    sF = vec_unpackh_s16(src7);
    STEP8(s0, s1, s2, s3, s4, s5, s6, s7, vec_4s);
    SHIFT_HOR8(s0, s1, s2, s3, s4, s5, s6, s7);
    STEP8(s8, s9, sA, sB, sC, sD, sE, sF, vec_4s);
    SHIFT_HOR8(s8, s9, sA, sB, sC, sD, sE, sF);
    src0 = vec_pack_s32(s8, s0);
    src1 = vec_pack_s32(s9, s1);
    src2 = vec_pack_s32(sA, s2);
    src3 = vec_pack_s32(sB, s3);
    src4 = vec_pack_s32(sC, s4);
    src5 = vec_pack_s32(sD, s5);
    src6 = vec_pack_s32(sE, s6);
    src7 = vec_pack_s32(sF, s7);
    TRANSPOSE8(src0, src1, src2, src3, src4, src5, src6, src7);

    s0 = vec_unpackl_s16(src0);
    s1 = vec_unpackl_s16(src1);
    s2 = vec_unpackl_s16(src2);
    s3 = vec_unpackl_s16(src3);
    s4 = vec_unpackl_s16(src4);
    s5 = vec_unpackl_s16(src5);
    s6 = vec_unpackl_s16(src6);
    s7 = vec_unpackl_s16(src7);
    s8 = vec_unpackh_s16(src0);
    s9 = vec_unpackh_s16(src1);
    sA = vec_unpackh_s16(src2);
    sB = vec_unpackh_s16(src3);
    sC = vec_unpackh_s16(src4);
    sD = vec_unpackh_s16(src5);
    sE = vec_unpackh_s16(src6);
    sF = vec_unpackh_s16(src7);
    STEP8(s0, s1, s2, s3, s4, s5, s6, s7, vec_64);
    SHIFT_VERT8(s0, s1, s2, s3, s4, s5, s6, s7);
    STEP8(s8, s9, sA, sB, sC, sD, sE, sF, vec_64);
    SHIFT_VERT8(s8, s9, sA, sB, sC, sD, sE, sF);
    src0 = vec_pack_s32(s8, s0);
    src1 = vec_pack_s32(s9, s1);
    src2 = vec_pack_s32(sA, s2);
    src3 = vec_pack_s32(sB, s3);
    src4 = vec_pack_s32(sC, s4);
    src5 = vec_pack_s32(sD, s5);
    src6 = vec_pack_s32(sE, s6);
    src7 = vec_pack_s32(sF, s7);

    vec_st(src0,  0, block);
    vec_st(src1, 16, block);
    vec_st(src2, 32, block);
    vec_st(src3, 48, block);
    vec_st(src4, 64, block);
    vec_st(src5, 80, block);
    vec_st(src6, 96, block);
    vec_st(src7,112, block);
}

#if 1
/** Do inverse transform on 8x4 part of block
*/
static void vc1_inv_trans_8x4_vmx(unsigned char *dest, int stride, DCTELEM *block)
{
/*
    vector signed short src0, src1, src2, src3, src4, src5, src6, src7;
    vector signed int s0, s1, s2, s3, s4, s5, s6, s7;
    vector signed int s8, s9, sA, sB, sC, sD, sE, sF;
    vector signed int t0, t1, t2, t3, t4, t5, t6, t7;
*/
	__vector4 src0, src1, src2, src3, src4, src5, src6, src7;//signed short 
    __vector4 s0, s1, s2, s3, s4, s5, s6, s7;		//signed int 
    __vector4 s8, s9, sA, sB, sC, sD, sE, sF;		//signed int 
    __vector4 t0, t1, t2, t3, t4, t5, t6, t7;		//signed int 
/*
    const vector signed int vec_64 = vec_sl(vec_splat_s32(4), vec_splat_u32(4));
    const vector unsigned int vec_7 = vec_splat_u32(7);
    const vector unsigned int vec_5 = vec_splat_u32(5);
    const vector unsigned int vec_4 = vec_splat_u32(4);
    const vector  signed int vec_4s = vec_splat_s32(4);
    const vector unsigned int vec_3 = vec_splat_u32(3);
    const vector unsigned int vec_2 = vec_splat_u32(2);
    const vector unsigned int vec_1 = vec_splat_u32(1);
*/
	__vector4 vec_64; //signed int 
    __vector4 vec_7; //unsigned int 
	__vector4 vec_5; //unsigned int 
    __vector4 vec_4; 	//unsigned int 
    __vector4 vec_4s; //signed int 
    __vector4 vec_3; 	//unsigned int 
    __vector4 vec_2; 	//unsigned int 
    __vector4 vec_1s; //signed int 
    __vector4 vec_1; 	//unsigned int 
/*	
	vector unsigned char tmp;
    vector signed short tmp2, tmp3;
    vector unsigned char perm0, perm1, p0, p1, p;
*/
	__vector4 tmp;				//unsigned char
    __vector4 tmp2, tmp3;		//signed short 
    __vector4 perm0, perm1, p0, p1, p;//unsigned char

	vec_64 = vec_sl_s32(vec_splat_s32(4), vec_splat_u32(4));//signed int 
    vec_7 = vec_splat_u32(7);		//unsigned int 
	vec_5 = vec_splat_u32(5);		//unsigned int 
    vec_4 = vec_splat_u32(4);		//unsigned int 
    vec_4s = vec_splat_s32(4);		//signed int 
    vec_3 = vec_splat_u32(3);		//unsigned int 
    vec_2 = vec_splat_u32(2);		//unsigned int 
    vec_1s = vec_splat_s32(1);		//signed int 
    vec_1 = vec_splat_u32(1);		//unsigned int 	

    src0 = vec_ld(  0, block);
    src1 = vec_ld( 16, block);
    src2 = vec_ld( 32, block);
    src3 = vec_ld( 48, block);
    src4 = vec_ld( 64, block);
    src5 = vec_ld( 80, block);
    src6 = vec_ld( 96, block);
    src7 = vec_ld(112, block);

    TRANSPOSE8(src0, src1, src2, src3, src4, src5, src6, src7);
    s0 = vec_unpackl_s16(src0);
    s1 = vec_unpackl_s16(src1);
    s2 = vec_unpackl_s16(src2);
    s3 = vec_unpackl_s16(src3);
    s4 = vec_unpackl_s16(src4);
    s5 = vec_unpackl_s16(src5);
    s6 = vec_unpackl_s16(src6);
    s7 = vec_unpackl_s16(src7);
    s8 = vec_unpackh_s16(src0);
    s9 = vec_unpackh_s16(src1);
    sA = vec_unpackh_s16(src2);
    sB = vec_unpackh_s16(src3);
    sC = vec_unpackh_s16(src4);
    sD = vec_unpackh_s16(src5);
    sE = vec_unpackh_s16(src6);
    sF = vec_unpackh_s16(src7);
    STEP8(s0, s1, s2, s3, s4, s5, s6, s7, vec_4s);
    SHIFT_HOR8(s0, s1, s2, s3, s4, s5, s6, s7);
    STEP8(s8, s9, sA, sB, sC, sD, sE, sF, vec_4s);
    SHIFT_HOR8(s8, s9, sA, sB, sC, sD, sE, sF);
    src0 = vec_pack_s32(s8, s0);
    src1 = vec_pack_s32(s9, s1);
    src2 = vec_pack_s32(sA, s2);
    src3 = vec_pack_s32(sB, s3);
    src4 = vec_pack_s32(sC, s4);
    src5 = vec_pack_s32(sD, s5);
    src6 = vec_pack_s32(sE, s6);
    src7 = vec_pack_s32(sF, s7);
    TRANSPOSE8(src0, src1, src2, src3, src4, src5, src6, src7);

    s0 = vec_unpackh_s16(src0);
    s1 = vec_unpackh_s16(src1);
    s2 = vec_unpackh_s16(src2);
    s3 = vec_unpackh_s16(src3);
    s8 = vec_unpackl_s16(src0);
    s9 = vec_unpackl_s16(src1);
    sA = vec_unpackl_s16(src2);
    sB = vec_unpackl_s16(src3);
    STEP4(s0, s1, s2, s3, vec_64);
    SHIFT_VERT4(s0, s1, s2, s3);
    STEP4(s8, s9, sA, sB, vec_64);
    SHIFT_VERT4(s8, s9, sA, sB);
    src0 = vec_pack_s32(s0, s8);
    src1 = vec_pack_s32(s1, s9);
    src2 = vec_pack_s32(s2, sA);
    src3 = vec_pack_s32(s3, sB);

    p0 = vec_lvsl (0, dest);
    p1 = vec_lvsl (stride, dest);
    p = vec_splat_u8 (-1);
    perm0 = vec_mergeh_u8 (p, p0);
    perm1 = vec_mergeh_u8 (p, p1);
	
	

#define ADD(dest,src,perm)                                              \
    /* *(uint64_t *)&tmp = *(uint64_t *)dest; */                        \
    tmp = vec_ld (0, dest);                                             \
    tmp2 = vec_perm (tmp, vec_splat_u8(0), perm);  \
    tmp3 = vec_adds_s16 (tmp2, src);                                    \
    tmp = vec_packsu (tmp3, tmp3);                                     \
    vec_ste_s32 (tmp, 0, (unsigned int *)dest);        \
    vec_ste_s32 (tmp, 4, (unsigned int *)dest);

    ADD (dest, src0, perm0)      dest += stride;
    ADD (dest, src1, perm1)      dest += stride;
    ADD (dest, src2, perm0)      dest += stride;
    ADD (dest, src3, perm1)
}

#endif
void vc1dsp_init_vmx(DSPContext* dsp, AVCodecContext *avctx) {
    dsp->vc1_inv_trans_8x8 = vc1_inv_trans_8x8_vmx;
   // dsp->vc1_inv_trans_8x4 = vc1_inv_trans_8x4_vmx;
}

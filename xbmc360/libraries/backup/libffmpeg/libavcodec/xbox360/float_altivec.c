/*
 * Copyright (c) 2006 Luca Barbato <lu_zero@gentoo.org>
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

#define DECLARE_ALIGNED(n,t,v)      __declspec(align(n)) t v
#undef malloc
#undef free

//Okay ..
static void vector_fmul_vmx(float *dst, const float *src, int len)
{
    int i;
    __vector4 d0, d1, s, zero = __vzero();//vspltisw 0
    for(i=0; i<len-7; i+=8) {
        d0 	= 	__lvx(dst+i, 0);
        s 	= 	__lvx(src+i, 0);
        d1 	= 	__lvx(dst+i, 16);
        d0 	= 	__vmaddfp(d0, s, zero);
        d1 	= 	__vmaddfp(d1, __lvx(src+i,16), zero);
        __stvx (d0, dst+i, 0 );
        __stvx (d1, dst+i, 16 );
    }
}
static void vector_fmul_c(float *dst, const float *src0, const float *src1, int len){
    int i;
    for(i=0; i<len; i++)
        dst[i] = src0[i] * src1[i];
}
static void vector_fmul_c_unroll(float *dst, const float *src0, const float *src1, int len){
    int i;
    for(i=0; i<len; i+=4)
	{
		dst[i+1] = src0[i+1] * src1[i+1];
		dst[i+2] = src0[i+2] * src1[i+2];
		dst[i+3] = src0[i+3] * src1[i+3];
		dst[i+4] = src0[i+4] * src1[i+4];
	}
}
//Up to date - 4 Operation per instruction !
static void vector_fmul_vmx2(float *dst, const float *src0, const float *src1, int len){
    int i;
	__vector4 vsrc0, vsrc1, vdst, zero = __vzero();//vspltisw 0
    for(i=0; i<len; i+8)// 8 = 128/sizeof(float)
    {
		vsrc0 = __lvx(src0+i, 0);//s0 = src0[i] // load 4 float
		vsrc1 = __lvx(src1+i, 0);//s1 = src1[i] // load 4 float
		//dst[i] = src0[i] * src1[i];
		vdst = __vmulfp(vsrc0,vsrc1);
		__stvx(vdst,dst,i);
	}
}
//Okay
static void vector_fmul_reverse_vmx(float *dst, const float *src0,
                                        const float *src1, int len)
{
    int i;
    __vector4 d, s0, s1, h0, l0,
                 s2, s3, zero = __vzero();//vspltisw 0
    src1 += len-4;
    for(i=0; i<len-7; i+=8) {
        s1 = __lvx(src1-i, 0);              // [a,b,c,d]
        s0 = __lvx(src0+i, 0 );
        l0 = __vmrglw(s1, s1);             // [c,c,d,d]
        s3 = __lvx(src1-i, -16);
        h0 = __vmrghw(s1, s1);             // [a,a,b,b]
        s2 = __lvx(src0+i, 16);
        s1 = __vmrghw(	__vmrglw(l0,h0),   // [d,b,d,b]
                        __vmrghw(l0,h0)	);  // [c,a,c,a]
                                             // [d,c,b,a]
        l0 = __vmrglw(s3, s3);
        d = __vmaddfp(s0, s1, zero);
        h0 = __vmrghw(s3, s3);
        __stvx(d, dst+i, 0);
        s3 = __vmrghw(	__vmrglw(l0,h0),
                        __vmrghw(l0,h0));
        d = __vmaddfp(s2, s3, zero);
        __stvx(d, dst+i, 16);
    }
}

//Okay
static void vector_fmul_add_vmx(float *dst, const float *src0,
                                    const float *src1, const float *src2,
                                    int len)
{
    int i;
	/*
    vector float d, s0, s1, s2, t0, t1, edges;
    vector unsigned char align = vec_lvsr(0,dst),
                         mask = vec_lvsl(0, dst);
	*/
	__vector4 d, s0, s1, s2, t0, t1, edges;//float
	__vector4 align,mask;

	align = __lvsr(dst,0);
	mask = __lvsl(dst,0);
	
    for (i=0; i<len-3; i+=4) {
        t0 = __lvx(dst+i, 0);
        t1 = __lvx(dst+i, 15);
        s0 = __lvx(src0+i, 0);
        s1 = __lvx(src1+i, 0);
        s2 = __lvx(src2+i, 0);
        edges = __vperm(t1 ,t0, mask);
        d = __vmaddfp(s0,s1,s2);
        t1 = __vperm(d, edges, align);
        t0 = __vperm(edges, d, align);
        __stvx(t1, dst+i, 15);
        __stvx(t0, dst+i, 0);
    }
}

//Okay
static void vector_fmul_window_vmx(float *dst, const float *src0, const float *src1, const float *win, float add_bias, int len)
{
/*
    union {
        vector float v;
        float s[4];
    } vadd;
	
    vector float vadd_bias, zero, t0, t1, s0, s1, wi, wj;
    const vector unsigned char reverse = vcprm(3,2,1,0);
    int i,j;

    dst += len;
    win += len;
    src0+= len;

    vadd.s[0] = add_bias;
    vadd_bias = vec_splat(vadd.v, 0);
    zero = __vzero();
*/
	int i,j;
	__vector4 vadd = {add_bias,0,0,0};
	__vector4 vadd_bias, zero, t0, t1, s0, s1, wi, wj;//float
	__vector4 reverse = {3,2,1,0}; //vcprm(3,2,1,0);
	
	dst += len;
    win += len;
    src0+= len;
		
	//vadd.v[0] = add_bias; //float
	
	vadd_bias = __vspltw(vadd, 0);
	
	zero = __vzero();
	
    for(i=-len*4, j=len*4-16; i<0; i+=16, j-=16) {
        s0 = __lvx(src0, i);
        s1 = __lvx(src1, j);
        wi = __lvx(win, i);
        wj = __lvx(win, j);

        s1 = __vperm(s1, s1, reverse);
        wj = __vperm(wj, wj, reverse);

        t0 = __vmaddfp(s0, wj, vadd_bias);
        t0 = __vnmsubfp(s1, wi, t0);
        t1 = __vmaddfp(s0, wi, vadd_bias);
        t1 = __vmaddfp(s1, wj, t1);
        t1 = __vperm(t1, t1, reverse);

        __stvx(t0, dst, i);
        __stvx(t1, dst, j);
    }
}

//Okay
static void int32_to_float_fmul_scalar_vmx(float *dst, const int *src, float mul, int len)
{
    __vector4 mul_u ;//= {mul,0,0,0};
    int i;
    __vector4 src1, src2, dst1, dst2, mul_v, zero;//float

    zero = __vzero();
    mul_u.v[0] = mul;
    mul_v = __vspltw(mul_u, 0);

	for(i=0; i<len; i+=8) {
       // src1 = vec_ctf(__lvx(src+i,0), 0);
       // src2 = vec_ctf(__lvx(src+i,16), 0);
		src1 = __vcfsx(__lvx(src+i,0), 0);
        src2 = __vcfsx(__lvx(src+i,16), 0);
        dst1 = __vmaddfp(src1, mul_v, zero);
        dst2 = __vmaddfp(src2, mul_v, zero);
        __stvx(dst1, dst+i, 0);
        __stvx(dst2, dst+i, 16);
    }
}

//Okay
static __vector4 float_to_int16_one_vmx(const float *src)
{
/*
    vector float s0 = vec_ld(0, src);
    vector float s1 = vec_ld(16, src);
    vector signed int t0 = vec_cts(s0, 0);
    vector signed int t1 = vec_cts(s1, 0);
*/
	__vector4 s0 = __lvx(src,0);
	__vector4 s1 = __lvx(src,16);
	__vector4 t0 = __vctsxs(s0,0);
	__vector4 t1 = __vctsxs(s1,16);
    return __vpkswss(t0,t1);
}

static void float_to_int16_vmx(int16_t *dst, const float *src, long len)
{
/*
    int i;
    vector signed short d0, d1, d;
    vector unsigned char align;
    if(((long)dst)&15) //FIXME
    for(i=0; i<len-7; i+=8) {
        d0 = vec_ld(0, dst+i);
        d = float_to_int16_one_vmx(src+i);
        d1 = vec_ld(15, dst+i);
        d1 = vec_perm(d1, d0, vec_lvsl(0,dst+i));
        align = vec_lvsr(0, dst+i);
        d0 = vec_perm(d1, d, align);
        d1 = vec_perm(d, d1, align);
        vec_st(d0, 0, dst+i);
        vec_st(d1,15, dst+i);
    }
    else
    for(i=0; i<len-7; i+=8) {
        d = float_to_int16_one_vmx(src+i);
        vec_st(d, 0, dst+i);
    }
*/
	int i;
    __vector4 d0, d1, d;//vector signed short
    __vector4 align;//unsigned char 
    if(((long)dst)&15) //FIXME
    for(i=0; i<len-7; i+=8) {
        d0 = __lvx(dst+i,0);
        d = float_to_int16_one_vmx(src+i);
        d1 = __lvx(dst+i,15);
        d1 = __vperm(d1, d0, __lvsl(dst+i,0));
        align = __lvsr(dst+i, 0);
        d0 = __vperm(d1, d, align);
        d1 = __vperm(d, d1, align);
        __stvx(d0, dst+i, 0 );
        __stvx(d1, dst+i, 15 );
    }
    else
    for(i=0; i<len-7; i+=8) {
        d = float_to_int16_one_vmx(src+i);
        __stvx(d0, dst+i, 0 );
    }
}

static void
float_to_int16_interleave_vmx(int16_t *dst, const float **src,
                                  long len, int channels)
{
/*
    int i;
    vector signed short d0, d1, d2, c0, c1, t0, t1;
    vector unsigned char align;
    if(channels == 1)
        float_to_int16_vmx(dst, src[0], len);
    else
        if (channels == 2) {
        if(((long)dst)&15)
        for(i=0; i<len-7; i+=8) {
            d0 = vec_ld(0, dst + i);
            t0 = float_to_int16_one_vmx(src[0] + i);
            d1 = vec_ld(31, dst + i);
            t1 = float_to_int16_one_vmx(src[1] + i);
            c0 = vec_mergeh(t0, t1);
            c1 = vec_mergel(t0, t1);
            d2 = vec_perm(d1, d0, vec_lvsl(0, dst + i));
            align = vec_lvsr(0, dst + i);
            d0 = vec_perm(d2, c0, align);
            d1 = vec_perm(c0, c1, align);
            vec_st(d0,  0, dst + i);
            d0 = vec_perm(c1, d2, align);
            vec_st(d1, 15, dst + i);
            vec_st(d0, 31, dst + i);
            dst+=8;
        }
        else
        for(i=0; i<len-7; i+=8) {
            t0 = float_to_int16_one_vmx(src[0] + i);
            t1 = float_to_int16_one_vmx(src[1] + i);
            d0 = vec_mergeh(t0, t1);
            d1 = vec_mergel(t0, t1);
            vec_st(d0,  0, dst + i);
            vec_st(d1, 16, dst + i);
            dst+=8;
        }
    } else {
        DECLARE_ALIGNED(16, int16_t, tmp)[len];
        int c, j;
        for (c = 0; c < channels; c++) {
            float_to_int16_vmx(tmp, src[c], len);
            for (i = 0, j = c; i < len; i++, j+=channels) {
                dst[j] = tmp[i];
            }
        }
   }
   
   */
   
	int i;
    __vector4  d0, d1, d2, c0, c1, t0, t1;//signed short
    __vector4  align;//unsigned char
	//DECLARE_ALIGNED(16, int16_t, tmp)[len];
    if(channels == 1)
        float_to_int16_vmx(dst, src[0], len);
    else
        if (channels == 2) {
        if(((long)dst)&15)
        for(i=0; i<len-7; i+=8) {
            d0 = __lvx(dst + i,0);
            t0 = float_to_int16_one_vmx(src[0] + i);
            d1 = __lvx(dst + i,31);
            t1 = float_to_int16_one_vmx(src[1] + i);
            c0 = __vmrghh(t0, t1);
            c1 = __vmrglh(t0, t1);
            d2 = __vperm(d1, d0, __lvsl(dst + i,0));//!!!!
            align = __lvsr(dst + i, 0);
            d0 = __vperm(d2, c0, align);
            d1 = __vperm(c0, c1, align);
            __stvx(d0, dst + i, 0);
            d0 = __vperm(c1, d2, align);
            __stvx(d1, dst + i, 15);
            __stvx(d0, dst + i, 31);
            dst+=8;
        }
        else
        for(i=0; i<len-7; i+=8) {
            t0 = float_to_int16_one_vmx(src[0] + i);
            t1 = float_to_int16_one_vmx(src[1] + i);
            d0 = __vmrghh(t0, t1);
            d1 = __vmrglh(t0, t1);
            __stvx(d0, dst + i, 0);
            __stvx(d1, dst + i, 16);
            dst+=8;
        }
    } else {
        int c, j;
        for (c = 0; c < channels; c++) {
			int16_t * tmp = (int16_t*)malloc(len*sizeof(int16_t));
			memset(tmp,0,len*sizeof(int16_t));
            float_to_int16_vmx(tmp, src[c], len);
            for (i = 0, j = c; i < len; i++, j+=channels) {
                dst[j] = tmp[i];
            }
			free(tmp);
        }
   }
}

void float_init_vmx(DSPContext* c, AVCodecContext *avctx)
{
    c->vector_fmul = vector_fmul_vmx;//no tested
    c->vector_fmul_reverse = vector_fmul_reverse_vmx;//working
	c->vector_fmul_add = vector_fmul_add_vmx;//working
    c->int32_to_float_fmul_scalar = int32_to_float_fmul_scalar_vmx;//no tested
    if(!(avctx->flags & CODEC_FLAG_BITEXACT)) {
     //   c->vector_fmul_window = vector_fmul_window_vmx;
     //   c->float_to_int16 = float_to_int16_vmx;
     //   c->float_to_int16_interleave = float_to_int16_interleave_vmx;
    }
}

#include <xtl.h>
#include "libavcodec/dsputil.h"
#include "libavcodec/h264data.h"
#include "libavcodec/h264dsp.h"


/*

a porter ...

Function Name				Inclusive 	Exclusive 	%Inclusive	% Exclusive	Comment
put_h264_chroma_mc8_c		2654		2654		12,63 %		12,63 %	
put_h264_qpel8_h_lowpass	1218		1218		5,80 %		5,80 %	
avg_h264_chroma_mc8_c		1191		1191		5,67 %		5,67 %	
h264_loop_filter_luma_c		1190		1190		5,66 %		5,66 %	
weight_h264_pixels16x16_c	1165		1165		5,54 %		5,54 %	
h264_loop_filter_chroma_c	708			708			3,37 %		3,37 %	

*/
/*
__vector4 Transform( __vector4 Vector, __vector4 Matrix[] )
{
    // __vmsum4fp == 4 element dot product
    __vector4 x = __vmsum4fp( Vector, Matrix[0] );
    __vector4 y = __vmsum4fp( Vector, Matrix[1] );
    __vector4 z = __vmsum4fp( Vector, Matrix[2] );
    __vector4 w = __vmsum4fp( Vector, Matrix[3] );
    // __vmrglw is Vector MeRGe Low Word
    __vector4 t1 = __vmrglw( x, z );    // t1 = { x, z, x, z };
    __vector4 t2 = __vmrglw( y, w );    // t2 = { y, w, y, w };
    __vector4 result  = __vmrglw( t1, t2 ); // { x, y, z, w };
    return result;
}
*/
//Macro ...

#define H264_MC(OPNAME, SIZE, CODETYPE) \
static void OPNAME ## h264_qpel ## SIZE ## _mc00_ ## CODETYPE (uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## pixels ## SIZE ## _ ## CODETYPE(dst, src, stride, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc10_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){ \
    DECLARE_ALIGNED(16, uint8_t, half)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(half, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, src, half, stride, stride, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc20_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(dst, src, stride, stride);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc30_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, half)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(half, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, src+1, half, stride, stride, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc01_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, half)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(half, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, src, half, stride, stride, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc02_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(dst, src, stride, stride);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc03_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, half)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(half, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, src+stride, half, stride, stride, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc11_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfH)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfV)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(halfH, src, SIZE, stride);\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(halfV, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfH, halfV, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc31_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfH)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfV)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(halfH, src, SIZE, stride);\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(halfV, src+1, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfH, halfV, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc13_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfH)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfV)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(halfH, src + stride, SIZE, stride);\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(halfV, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfH, halfV, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc33_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfH)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfV)[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(halfH, src + stride, SIZE, stride);\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(halfV, src+1, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfH, halfV, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc22_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, int16_t, tmp)[SIZE*(SIZE+8)];\
    OPNAME ## h264_qpel ## SIZE ## _hv_lowpass_ ## CODETYPE(dst, tmp, src, stride, SIZE, stride);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc21_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfH)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfHV)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, int16_t, tmp)[SIZE*(SIZE+8)];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(halfH, src, SIZE, stride);\
    put_h264_qpel ## SIZE ## _hv_lowpass_ ## CODETYPE(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfH, halfHV, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc23_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfH)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfHV)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, int16_t, tmp)[SIZE*(SIZE+8)];\
    put_h264_qpel ## SIZE ## _h_lowpass_ ## CODETYPE(halfH, src + stride, SIZE, stride);\
    put_h264_qpel ## SIZE ## _hv_lowpass_ ## CODETYPE(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfH, halfHV, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc12_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfV)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfHV)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, int16_t, tmp)[SIZE*(SIZE+8)];\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(halfV, src, SIZE, stride);\
    put_h264_qpel ## SIZE ## _hv_lowpass_ ## CODETYPE(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfV, halfHV, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc32_ ## CODETYPE(uint8_t *dst, uint8_t *src, int stride){\
    DECLARE_ALIGNED(16, uint8_t, halfV)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, uint8_t, halfHV)[SIZE*SIZE];\
    DECLARE_ALIGNED(16, int16_t, tmp)[SIZE*(SIZE+8)];\
    put_h264_qpel ## SIZE ## _v_lowpass_ ## CODETYPE(halfV, src+1, SIZE, stride);\
    put_h264_qpel ## SIZE ## _hv_lowpass_ ## CODETYPE(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2_ ## CODETYPE(dst, halfV, halfHV, stride, SIZE, SIZE);\
}\


static inline void put_pixels16_l2_altivec( uint8_t * dst, const uint8_t * src1,
                                    const uint8_t * src2, int dst_stride,
                                    int src_stride1, int h)
{
    int i;
	__vector4 a, b, d, tmp1, tmp2, mask, mask_, edges, align; //u8

    mask_ = __lvsl(src2, 0);

    for (i = 0; i < h; i++) {

		tmp1 = __lvx(src1, i * src_stride1);
        mask = __lvsl(src1, i * src_stride1);
        tmp2 = __lvx(src1, i * src_stride1 + 15);

        a = __vperm(tmp1, tmp2, mask);

        tmp1 = __lvx(src2,i * 16);
        tmp2 = __lvx(src2,i * 16 + 15);

        b = __vperm(tmp1, tmp2, mask_);

        tmp1 = __lvx(dst, 0);
        mask = __lvsl(dst, 0);
        tmp2 = __lvx(dst, 15);

        d = __vavgub(a, b);

        edges = __vperm(tmp2, tmp1, mask);

        align = __lvsr(dst, 0);

        tmp2 = __vperm(d, edges, align);
        tmp1 = __vperm(edges, d, align);

        __stvx(tmp2, dst, 15);
        __stvx(tmp1, dst, 0);

        dst += dst_stride;
    }
}


static inline void avg_pixels16_l2_altivec( uint8_t * dst, const uint8_t * src1,
                                    const uint8_t * src2, int dst_stride,
                                    int src_stride1, int h)
{
    int i;
    __vector4 a, b, d, tmp1, tmp2, mask, mask_, edges, align;

    mask_ = __lvsl(src2, 0);

    for (i = 0; i < h; i++) {

        tmp1 = __lvx(src1, i * src_stride1);
        mask = __lvsl(src1, i * src_stride1);
        tmp2 = __lvx(src1, i * src_stride1 + 15);

        a = __vperm(tmp1, tmp2, mask);

        tmp1 = __lvx(src2, i * 16);
        tmp2 = __lvx(src2, i * 16 + 15);

        b = __vperm(tmp1, tmp2, mask_);

        tmp1 = __lvx(dst, 0);
        mask = __lvsl(dst, 0);
        tmp2 = __lvx(dst, 15);

        d = __vavgub(__vperm(tmp1, tmp2, mask), __vavgub(a, b));

        edges = __vperm(tmp2, tmp1, mask);

        align = __lvsr(dst, 0);

        tmp2 = __vperm(d, edges, align);
        tmp1 = __vperm(edges, d, align);

        __stvx(tmp2, dst, 15);
        __stvx(tmp1, dst, 0);

        dst += dst_stride;
    }
}

/*
H264_MC(put_, 16, altivec)
H264_MC(avg_, 16, altivec)
*/



#define vec_mergeh_s16(a,b)	__vmrghh(a,b)
#define vec_ld(a,b)			__lvx(b,a)
#define vec_perm			__vperm
#define vec_sld(a,b,c)		__vsldoi(a,b,c)
#define vec_lvsl(a,b)		__lvsl(b,a);

#define vec_add				__vadduhm
#define vec_sub				__vsubuhm 
#define	vec_sra				__vsrah
#define vec_splat_u16(a)	__vsplth(__vspltisw(1),a)

#define vec_mergeh_u8		__vmrghb
#define vec_ste_u32(v,o,a)	__stvewx(v,a,o)			
#define vec_packsu			__vpkshus
#define vec_splat_u32(a,b)	__vspltw(a,b)

typedef __vector4 vec_s16;
typedef __vector4 vec_u8;
typedef __vector4 vec_u16;
typedef __vector4 vec_u32;

/****************************************************************************
 * IDCT transform:
 ****************************************************************************/
#define VEC_1D_DCT(vb0,vb1,vb2,vb3,va0,va1,va2,va3)               \
    /* 1st stage */                                               \
    vz0 = vec_add(vb0,vb2);       /* temp[0] = Y[0] + Y[2] */     \
    vz1 = vec_sub(vb0,vb2);       /* temp[1] = Y[0] - Y[2] */     \
    vz2 = vec_sra(vb1,vec_splat_u16(1));                          \
    vz2 = vec_sub(vz2,vb3);       /* temp[2] = Y[1].1/2 - Y[3] */ \
    vz3 = vec_sra(vb3,vec_splat_u16(1));                          \
    vz3 = vec_add(vb1,vz3);       /* temp[3] = Y[1] + Y[3].1/2 */ \
    /* 2nd stage: output */                                       \
    va0 = vec_add(vz0,vz3);       /* x[0] = temp[0] + temp[3] */  \
    va1 = vec_add(vz1,vz2);       /* x[1] = temp[1] + temp[2] */  \
    va2 = vec_sub(vz1,vz2);       /* x[2] = temp[1] - temp[2] */  \
    va3 = vec_sub(vz0,vz3)        /* x[3] = temp[0] - temp[3] */

#define VEC_TRANSPOSE_4_S16(a0,a1,a2,a3,b0,b1,b2,b3) \
    b0 = vec_mergeh_s16( a0, a0 ); \
    b1 = vec_mergeh_s16( a1, a0 ); \
    b2 = vec_mergeh_s16( a2, a0 ); \
    b3 = vec_mergeh_s16( a3, a0 ); \
    a0 = vec_mergeh_s16( b0, b2 ); \
    a1 = vec_mergeh_s16( b0, b2 ); \
    a2 = vec_mergeh_s16( b1, b3 ); \
    a3 = vec_mergeh_s16( b1, b3 ); \
    b0 = vec_mergeh_s16( a0, a2 ); \
    b1 = vec_mergeh_s16( a0, a2 ); \
    b2 = vec_mergeh_s16( a1, a3 ); \
    b3 = vec_mergeh_s16( a1, a3 )

#define VEC_LOAD_U8_ADD_S16_STORE_U8(va)                      \
    vdst_orig = vec_ld(0, dst);                               \
    vdst = vec_perm(vdst_orig, zero_u8v, vdst_mask);          \
    vdst_ss = vec_mergeh_u8(__vzero(), vdst);         \
    va = vec_add(va, vdst_ss);                                \
    va_u8 = vec_packsu(va, __vzero());                        \
    va_u32 = vec_splat_u32(va_u8, 0);                  \
    vec_ste_u32(va_u32, element, (uint32_t*)dst);

static void ff_h264_idct_add_altivec(uint8_t *dst, DCTELEM *block, int stride)
{
#if 0
    vec_s16 va0, va1, va2, va3;
    vec_s16 vz0, vz1, vz2, vz3;
    vec_s16 vtmp0, vtmp1, vtmp2, vtmp3;
    vec_u8 va_u8;
    vec_u32 va_u32;
    vec_s16 vdst_ss;
    //const vec_u16 v6us = vec_splat_u16(6);
	vec_u16 v6us;
    vec_u8 vdst, vdst_orig;
    vec_u8 vdst_mask;

	vec_u8 zerov;
	vec_u8 zero_u8v;//= vec_splat_u8( 0 )
	int element = ((unsigned long)dst & 0xf) >> 2;

	v6us = vec_splat_u16(6);
	vdst_mask = vec_lvsl(0, dst);
	zerov = __vzero();
	zero_u8v = __vzero();
    
    //LOAD_ZERO;

    block[0] += 32;  /* add 32 as a DC-level for rounding */

    vtmp0 = vec_ld(0,block);
    vtmp1 = vec_sld(vtmp0, vtmp0, 8);
    vtmp2 = vec_ld(16,block);
    vtmp3 = vec_sld(vtmp2, vtmp2, 8);

    VEC_1D_DCT(vtmp0,vtmp1,vtmp2,vtmp3,va0,va1,va2,va3);
    VEC_TRANSPOSE_4_S16(va0,va1,va2,va3,vtmp0,vtmp1,vtmp2,vtmp3);
    VEC_1D_DCT(vtmp0,vtmp1,vtmp2,vtmp3,va0,va1,va2,va3);

    va0 = vec_sra(va0,v6us);
    va1 = vec_sra(va1,v6us);
    va2 = vec_sra(va2,v6us);
    va3 = vec_sra(va3,v6us);

    VEC_LOAD_U8_ADD_S16_STORE_U8(va0);
    dst += stride;
    VEC_LOAD_U8_ADD_S16_STORE_U8(va1);
    dst += stride;
    VEC_LOAD_U8_ADD_S16_STORE_U8(va2);
    dst += stride;
    VEC_LOAD_U8_ADD_S16_STORE_U8(va3);
#endif
}


void ff_h264dsp_init_ppc(H264DSPContext *c){
	
};
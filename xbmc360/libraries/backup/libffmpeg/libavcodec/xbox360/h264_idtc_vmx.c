#include <xtl.h>

#include "libavcodec/dsputil.h"
#include "libavcodec/h264data.h"
#include "libavcodec/h264dsp.h"


#if 0
/****************************************************************************
 * IDCT transform:
 ****************************************************************************/
#define VEC_1D_DCT(vb0,vb1,vb2,vb3,va0,va1,va2,va3)               \
    /* 1st stage */                                               \
    vz0 = __vadduhm(vb0,vb2);       /* temp[0] = Y[0] + Y[2] */     \
    vz1 = __vsubuhm(vb0,vb2);       /* temp[1] = Y[0] - Y[2] */     \
    vz2 = __vsrah(vb1,__vspltish(1));                          \
    vz2 = __vsubuhm(vz2,vb3);       /* temp[2] = Y[1].1/2 - Y[3] */ \
    vz3 = __vsrah (vb3,__vspltish(1));                          \
    vz3 = __vadduhm(vb1,vz3);       /* temp[3] = Y[1] + Y[3].1/2 */ \
    /* 2nd stage: output */                                       \
    va0 = __vadduhm (vz0,vz3);       /* x[0] = temp[0] + temp[3] */  \
    va1 = __vadduhm (vz1,vz2);       /* x[1] = temp[1] + temp[2] */  \
    va2 = __vsubuhm (vz1,vz2);       /* x[2] = temp[1] - temp[2] */  \
    va3 = __vsubuhm(vz0,vz3)        /* x[3] = temp[0] - temp[3] */
	
	
#define VEC_TRANSPOSE_4(a0,a1,a2,a3,b0,b1,b2,b3) \
    b0 = __vmrghh( a0, a0 ); \
    b1 = __vmrghh( a1, a0 ); \
    b2 = __vmrghh( a2, a0 ); \
    b3 = __vmrghh( a3, a0 ); \
    a0 = __vmrghh( b0, b2 ); \
    a1 = __vmrglh( b0, b2 ); \
    a2 = __vmrghh( b1, b3 ); \
    a3 = __vmrglh( b1, b3 ); \
    b0 = __vmrghh( a0, a2 ); \
    b1 = __vmrglh( a0, a2 ); \
    b2 = __vmrghh( a1, a3 ); \
    b3 = __vmrglh( a1, a3 )

#define VEC_LOAD_U8_ADD_S16_STORE_U8(va)            \
    vdst_orig = __lvx(dst, 0);						\
    vdst = __vperm(vdst_orig, __vzero(), vdst_mask);\
    vdst_ss = __vmrghb(__vzero(), vdst);			\
    va = __vaddshs(va, vdst_ss);                    \
    va_u8 = __vpkshus(va, __vzero());               \
    va_u32 = __vspltw(va_u8, 0);					\
    __stvewx(va_u32, (uint32_t*)dst, element);

//fucked :s
static void ff_h264_idct_add_altivec(uint8_t *dst, DCTELEM *block, int stride)
{
	
	__vector4 va0, va1, va2, va3;//s16
    __vector4 vz0, vz1, vz2, vz3;//s16
    __vector4 vtmp0, vtmp1, vtmp2, vtmp3;//s16
    __vector4 va_u8;//u8
    __vector4 va_u32;//u32
    __vector4 vdst_ss;//s16
    const __vector4 v6us = __vspltish(6);//u16
    __vector4 vdst, vdst_orig;//u8
    __vector4 vdst_mask = __lvsl(dst,0);//u8
    int element = ((unsigned long)dst & 0xf) >> 2;
    //LOAD_ZERO;
    block[0] += 32;  /* add 32 as a DC-level for rounding */

    vtmp0 = __lvx(block,0);
    vtmp1 = __vsldoi(vtmp0, vtmp0, 8);
    vtmp2 = __lvx(block,16);
    vtmp3 = __vsldoi(vtmp2, vtmp2, 8);

    VEC_1D_DCT(vtmp0,vtmp1,vtmp2,vtmp3,va0,va1,va2,va3);
    VEC_TRANSPOSE_4(va0,va1,va2,va3,vtmp0,vtmp1,vtmp2,vtmp3);
    VEC_1D_DCT(vtmp0,vtmp1,vtmp2,vtmp3,va0,va1,va2,va3);

    va0 = __vsrah(va0,v6us);
    va1 = __vsrah(va1,v6us);
    va2 = __vsrah(va2,v6us);
    va3 = __vsrah(va3,v6us);

    VEC_LOAD_U8_ADD_S16_STORE_U8(va0);
    dst += stride;
    VEC_LOAD_U8_ADD_S16_STORE_U8(va1);
    dst += stride;
    VEC_LOAD_U8_ADD_S16_STORE_U8(va2);
    dst += stride;
    VEC_LOAD_U8_ADD_S16_STORE_U8(va3);
}

#endif


//+ 5% on each thread !!!
void ff_h264_idct_init_test_ppc(H264DSPContext *c){
	/* Not working ...
	c->h264_idct_add = ff_h264_idct_add_altivec;
	c->h264_idct_add8 = ff_h264_idct_add8_altivec;
	c->h264_idct_add16 = ff_h264_idct_add16_altivec;
	c->h264_idct_add16intra = ff_h264_idct_add16intra_altivec;
	*/
}
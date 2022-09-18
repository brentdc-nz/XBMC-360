#include <xtl.h>

#include "libavcodec/dsputil.h"
#include "libavcodec/h264data.h"
#include "libavcodec/h264dsp.h"


/**
* 28/04/2011: fixed some intrinsics (u8 instead of s16). not tested
* Working :D
*/
static av_always_inline void h264_idct_dc_add_internal(uint8_t *dst, DCTELEM *block, int stride, int size)
{
	__vector4 dc16;//s16
	__vector4 dcplus, dcminus, v0, v1, v2, v3, aligner;//u8
	//LOAD_ZERO;
	int i;
	DECLARE_ALIGNED(16, int, dc);


	dc = (block[0] + 32) >> 6;
	/*
	lvewx   %vr1, %r0, %r9
	vsplth  %vr1, %vr1, 1
	lvx     %vr11, %r0, %r3
	*/
	dc16 = __vsplth(__lvewx(&dc, 0), 1);//vec_splat((vec_s16) vec_lde(0, &dc), 1)

	/*
	vsldoi  %vr1, %vr1, %vr13, 8
	*/
	if (size == 4)
		dc16 = __vsldoi(dc16, __vzero(), 8);

	dcplus = __vpkshus (dc16, __vzero());//vu8  = vec_packsu(vs16,vs16);
	dcminus = __vpkshus(__vsubuhm(__vzero(), dc16), __vzero());

	/*
	lvsr    %vr0, %r0, %r3
	*/
	aligner = __lvsr(dst,0);
	dcplus = __vperm(dcplus, dcplus, aligner);
	dcminus = __vperm(dcminus, dcminus, aligner);

	for (i = 0; i < size; i += 4) {
		v0 = __lvx(dst+0*stride, 0);
		v1 = __lvx(dst+1*stride, 0);
		v2 = __lvx(dst+2*stride, 0);
		v3 = __lvx(dst+3*stride, 0);

		v0 = __vaddubs(v0, dcplus);
		v1 = __vaddubs(v1, dcplus);
		v2 = __vaddubs(v2, dcplus);
		v3 = __vaddubs(v3, dcplus);

		v0 = __vsububs(v0, dcminus);
		v1 = __vsububs(v1, dcminus);
		v2 = __vsububs(v2, dcminus);
		v3 = __vsububs(v3, dcminus);

		__stvx(v0, dst+0*stride, 0);
		__stvx(v1, dst+1*stride, 0);
		__stvx(v2, dst+2*stride, 0);
		__stvx(v3, dst+3*stride, 0);

		dst += 4*stride;
	}
}


#define IDCT8_1D_ALTIVEC(\
	s0, s1, s2, s3, \
	s4, s5, s6, s7, \
	d0, d1, d2, d3, \
	d4, d5, d6, d7 \
	)  \
	do{ \
	__vector4 a0v = __vadduhm(s0, s4);     \
	__vector4 a2v = __vsubuhm(s0, s4);     \
	__vector4 a4v = __vsubuhm(__vsrah(s2, onev), s6);     \
	__vector4 a6v = __vadduhm(__vsrah(s6, onev), s2);     \
	__vector4 b0v = __vadduhm(a0v, a6v);   \
	__vector4 b2v = __vadduhm(a2v, a4v);   \
	__vector4 b4v = __vsubuhm(a2v, a4v);   \
	__vector4 b6v = __vsubuhm(a0v, a6v);   \
	__vector4 a1v = __vsubuhm( __vsubuhm(s5, s3), __vadduhm(s7, __vsrah(s7, onev)) );  \
	__vector4 a3v = __vsubuhm( __vadduhm(s7, s1), __vadduhm(s3, __vsrah(s3, onev)) ); \
	__vector4 a5v = __vadduhm( __vsubuhm(s7, s1), __vadduhm(s5, __vsrah(s5, onev)) ); \
	__vector4 a7v = __vadduhm( __vadduhm(s5, s3), __vadduhm(s1, __vsrah(s1, onev)) ); \
	__vector4 b1v = __vadduhm( __vsrah(a7v, twov), a1v);  \
	__vector4 b3v = __vadduhm(a3v, __vsrah(a5v, twov));  \
	__vector4 b5v = __vsubuhm( __vsrah(a3v, twov), a5v);  \
	__vector4 b7v = __vsubuhm( a7v, __vsrah(a1v, twov));  \
	d0 = __vadduhm(b0v, b7v);  \
	d1 = __vadduhm(b2v, b5v);  \
	d2 = __vadduhm(b4v, b3v);  \
	d3 = __vadduhm(b6v, b1v);  \
	d4 = __vsubuhm(b6v, b1v);  \
	d5 = __vsubuhm(b4v, b3v);  \
	d6 = __vsubuhm(b2v, b5v);  \
	d7 = __vsubuhm(b0v, b7v);  \
	} \
	while(0)

#define altivec_store_sum_clip(dest, idctv, perm_ldv, perm_stv, sel) \
	do{ \
	__vector4 sixv = __vspltish(6);							\
	\
	__vector4 hv = __lvx( dest, 0 );      					\
	__vector4 lv = __lvx( dest, 7 );        				\
	__vector4 dstv   = __vperm( hv, lv, perm_ldv );  		\
	__vector4 idct_sh6 = __vsrah(idctv, sixv);     			\
	__vector4 dst16 = __vmrghb(__vzero(), dstv);   			\
	__vector4 idstsum = __vaddshs(idct_sh6, dst16);  		\
	__vector4 idstsum8 = __vpkshus(__vzero(), idstsum);  	\
	__vector4 edgehv;                                    	\
	__vector4 bodyv  = __vperm( idstsum8, idstsum8, perm_stv );	\
	__vector4 edgelv = __vperm( sel, __vzero(), perm_stv ); \
	lv = __vsel( lv, bodyv, edgelv );      					\
	__stvx( lv, dest, 7 );           						\
	hv = __lvx( dest, 0 );                 					\
	edgehv = __vperm( __vzero(), sel, perm_stv );    		\
	hv = __vsel( hv, bodyv, edgehv );           			\
	__stvx( hv, dest, 0 );        							\
	}														\
	while(0);

// Transpose 8x8 matrix of 16-bit elements (in-place)
#define TRANSPOSE8(a,b,c,d,e,f,g,h) \
	do { \
	__vector4 A1, B1, C1, D1, E1, F1, G1, H1; \
	__vector4 A2, B2, C2, D2, E2, F2, G2, H2; \
	\
	A1 = __vmrghh (a, e); \
	B1 = __vmrglh (a, e); \
	C1 = __vmrghh (b, f); \
	D1 = __vmrglh (b, f); \
	E1 = __vmrghh (c, g); \
	F1 = __vmrglh (c, g); \
	G1 = __vmrghh (d, h); \
	H1 = __vmrglh (d, h); \
	\
	A2 = __vmrghh (A1, E1); \
	B2 = __vmrglh (A1, E1); \
	C2 = __vmrghh (B1, F1); \
	D2 = __vmrglh (B1, F1); \
	E2 = __vmrghh (C1, G1); \
	F2 = __vmrglh (C1, G1); \
	G2 = __vmrghh (D1, H1); \
	H2 = __vmrglh (D1, H1); \
	\
	a = __vmrghh (A2, E2); \
	b = __vmrglh (A2, E2); \
	c = __vmrghh (B2, F2); \
	d = __vmrglh (B2, F2); \
	e = __vmrghh (C2, G2); \
	f = __vmrglh (C2, G2); \
	g = __vmrghh (D2, H2); \
	h = __vmrglh (D2, H2); \
	} while (0);

//working :D
static void ff_h264_idct8_add_altivec( uint8_t *dst, DCTELEM *dct, int stride ) {
	__vector4 s0, s1, s2, s3, s4, s5, s6, s7;//s16
	__vector4 d0, d1, d2, d3, d4, d5, d6, d7;//s16
	__vector4 idct0, idct1, idct2, idct3, idct4, idct5, idct6, idct7;//s16

	__vector4 perm_ldv = __lvsl(dst,0);//u8
	__vector4 perm_stv = __lvsr(dst, 8);//u8

	//need to find something else ...
	__declspec(align(16)) signed char s8sel[] = {
		0,0,0,0,
		0,0,0,0,
		-1,-1,-1,-1,
		-1,-1,-1,-1
	};//u8

	__vector4 onev = __vspltish(1);//u16
	__vector4 twov = __vspltish(2);//u16
	__vector4 sixv = __vspltish(6);//u16

	__vector4 sel = __lvx(s8sel,0);

	//LOAD_ZERO;

	dct[0] += 32; // rounding for the >>6 at the end

	s0 = __lvx((int16_t*)dct, 0x00);
	s1 = __lvx((int16_t*)dct, 0x10);
	s2 = __lvx((int16_t*)dct, 0x20);
	s3 = __lvx((int16_t*)dct, 0x30);
	s4 = __lvx((int16_t*)dct, 0x40);
	s5 = __lvx((int16_t*)dct, 0x50);
	s6 = __lvx((int16_t*)dct, 0x60);
	s7 = __lvx((int16_t*)dct, 0x70);

	IDCT8_1D_ALTIVEC(s0, s1, s2, s3, s4, s5, s6, s7,
		d0, d1, d2, d3, d4, d5, d6, d7);

	TRANSPOSE8( d0,  d1,  d2,  d3,  d4,  d5,  d6, d7 );

	IDCT8_1D_ALTIVEC(d0,  d1,  d2,  d3,  d4,  d5,  d6, d7,
		idct0, idct1, idct2, idct3, idct4, idct5, idct6, idct7);

	altivec_store_sum_clip(&dst[0*stride], idct0, perm_ldv, perm_stv, sel);
	altivec_store_sum_clip(&dst[1*stride], idct1, perm_ldv, perm_stv, sel);
	altivec_store_sum_clip(&dst[2*stride], idct2, perm_ldv, perm_stv, sel);
	altivec_store_sum_clip(&dst[3*stride], idct3, perm_ldv, perm_stv, sel);
	altivec_store_sum_clip(&dst[4*stride], idct4, perm_ldv, perm_stv, sel);
	altivec_store_sum_clip(&dst[5*stride], idct5, perm_ldv, perm_stv, sel);
	altivec_store_sum_clip(&dst[6*stride], idct6, perm_ldv, perm_stv, sel);
	altivec_store_sum_clip(&dst[7*stride], idct7, perm_ldv, perm_stv, sel);
}


static void h264_idct_dc_add_altivec(uint8_t *dst, DCTELEM *block, int stride)
{
	h264_idct_dc_add_internal(dst, block, stride, 4);
}

static void ff_h264_idct8_dc_add_altivec(uint8_t *dst, DCTELEM *block, int stride)
{
	h264_idct_dc_add_internal(dst, block, stride, 8);
}


static void ff_h264_idct8_add4_altivec(uint8_t *dst, const int *block_offset, DCTELEM *block, int stride, const uint8_t nnzc[6*8]){
	int i;
	for(i=0; i<16; i+=4){
		int nnz = nnzc[ scan8[i] ];
		if(nnz){
			if(nnz==1 && block[i*16]) 
				ff_h264_idct8_dc_add_altivec(dst + block_offset[i], block + i*16, stride);
			else                      
				ff_h264_idct8_add_altivec   (dst + block_offset[i], block + i*16, stride);
		}
	}
}




void ff_h264_idct_init_test_ppc(H264DSPContext *c);

/*****************************
*	+ 5% on each thread !!!
*****************************/
void ff_h264_idct_init_ppc(H264DSPContext *c){
	//ff_h264_idct_init_test_ppc(c);
	c->h264_idct_dc_add= h264_idct_dc_add_altivec;
	c->h264_idct8_dc_add = ff_h264_idct8_dc_add_altivec;
	c->h264_idct8_add4 = ff_h264_idct8_add4_altivec;
}
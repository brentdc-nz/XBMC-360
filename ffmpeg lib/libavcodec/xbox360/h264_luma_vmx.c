#include <xtl.h>
#include "libavcodec/dsputil.h"
#include "libavcodec/h264data.h"
#include "libavcodec/h264dsp.h"


/**
* Inline
**/
static inline void write16x4(uint8_t *dst, int dst_stride,
							 register __vector4 r0, register __vector4 r1,
                             register __vector4 r2, register __vector4 r3) {
    DECLARE_ALIGNED(16, unsigned char, result)[64];
    uint32_t *src_int = (uint32_t *)result, *dst_int = (uint32_t *)dst;
    int int_dst_stride = dst_stride/4;

	__stvx(r0, result, 0 );
    __stvx(r1, result, 16);
    __stvx(r2, result, 32);
    __stvx(r3, result, 48);
    /* FIXME: there has to be a better way!!!! */
    *dst_int = *src_int;
    *(dst_int+   int_dst_stride) = *(src_int + 1);
    *(dst_int+ 2*int_dst_stride) = *(src_int + 2);
    *(dst_int+ 3*int_dst_stride) = *(src_int + 3);
    *(dst_int+ 4*int_dst_stride) = *(src_int + 4);
    *(dst_int+ 5*int_dst_stride) = *(src_int + 5);
    *(dst_int+ 6*int_dst_stride) = *(src_int + 6);
    *(dst_int+ 7*int_dst_stride) = *(src_int + 7);
    *(dst_int+ 8*int_dst_stride) = *(src_int + 8);
    *(dst_int+ 9*int_dst_stride) = *(src_int + 9);
    *(dst_int+10*int_dst_stride) = *(src_int + 10);
    *(dst_int+11*int_dst_stride) = *(src_int + 11);
    *(dst_int+12*int_dst_stride) = *(src_int + 12);
    *(dst_int+13*int_dst_stride) = *(src_int + 13);
    *(dst_int+14*int_dst_stride) = *(src_int + 14);
    *(dst_int+15*int_dst_stride) = *(src_int + 15);
}

#if 1

/** \brief loads unaligned vector \a *src with offset \a offset
    and returns it */
static inline __vector4 unaligned_load(int offset, uint8_t *src)
{
	/*
	register __vector4 first = __lvx(src,offset);
    register __vector4 second = __lvx(src, offset+15);
	register __vector4 mask = __lvsl(src, offset);
    return __vperm(first, second, mask);
	*/
	return __vor(__lvlx(src, offset), __lvrx(src,offset+16));
}

#endif

/** \brief performs a 6x16 transpose of data in src, and stores it to dst
    \todo FIXME: see if we can't spare some vec_lvsl() by them factorizing
    out of unaligned_load() */
static inline void  readAndTranspose16x6(
	uint8_t * src, 
	int src_stride,
	register __vector4 r8, 
	register __vector4 r9,
	register __vector4 r10, 
	register __vector4 r11,
	register __vector4 r12,
	register __vector4 r13
)
{
	register __vector4 r0  = unaligned_load(0,             src);            
    register __vector4 r1  = unaligned_load(   src_stride, src);            
    register __vector4 r2  = unaligned_load(2* src_stride, src);            
    register __vector4 r3  = unaligned_load(3* src_stride, src);            
    register __vector4 r4  = unaligned_load(4* src_stride, src);            
    register __vector4 r5  = unaligned_load(5* src_stride, src);            
    register __vector4 r6  = unaligned_load(6* src_stride, src);            
    register __vector4 r7  = unaligned_load(7* src_stride, src);            
    register __vector4 r14 = unaligned_load(14*src_stride, src);            
    register __vector4 r15 = unaligned_load(15*src_stride, src);            
                                                                         
    r8  = unaligned_load( 8*src_stride, src);                            
    r9  = unaligned_load( 9*src_stride, src);                            
    r10 = unaligned_load(10*src_stride, src);                              
    r11 = unaligned_load(11*src_stride, src);                              
    r12 = unaligned_load(12*src_stride, src);                              
    r13 = unaligned_load(13*src_stride, src);                              
                                                                           
    /*Merge first pairs*/                                                  
    r0 = __vmrghb(r0, r8);    /*0, 8*/                                   
    r1 = __vmrghb(r1, r9);    /*1, 9*/                                   
    r2 = __vmrghb(r2, r10);   /*2,10*/                                   
    r3 = __vmrghb(r3, r11);   /*3,11*/                                   
    r4 = __vmrghb(r4, r12);   /*4,12*/                                   
    r5 = __vmrghb(r5, r13);   /*5,13*/                                   
    r6 = __vmrghb(r6, r14);   /*6,14*/                                   
    r7 = __vmrghb(r7, r15);   /*7,15*/                                   
                                                                           
    /*Merge second pairs*/                                                 
    r8  = __vmrghb(r0, r4);   /*0,4, 8,12 set 0*/                        
    r9  = __vmrglb(r0, r4);   /*0,4, 8,12 set 1*/                        
    r10 = __vmrghb(r1, r5);   /*1,5, 9,13 set 0*/                        
    r11 = __vmrglb(r1, r5);   /*1,5, 9,13 set 1*/                        
    r12 = __vmrghb(r2, r6);   /*2,6,10,14 set 0*/                        
    r13 = __vmrglb(r2, r6);   /*2,6,10,14 set 1*/                        
    r14 = __vmrghb(r3, r7);   /*3,7,11,15 set 0*/                        
    r15 = __vmrglb(r3, r7);   /*3,7,11,15 set 1*/                        
                                                                           
    /*Third merge*/                                                        
    r0 = __vmrghb(r8,  r12);  /*0,2,4,6,8,10,12,14 set 0*/               
    r1 = __vmrglb(r8,  r12);  /*0,2,4,6,8,10,12,14 set 1*/               
    r2 = __vmrghb(r9,  r13);  /*0,2,4,6,8,10,12,14 set 2*/               
    r4 = __vmrghb(r10, r14);  /*1,3,5,7,9,11,13,15 set 0*/               
    r5 = __vmrglb(r10, r14);  /*1,3,5,7,9,11,13,15 set 1*/               
    r6 = __vmrghb(r11, r15);  /*1,3,5,7,9,11,13,15 set 2*/               
    /* Don't need to compute 3 and 7*/                                     
                                                                           
    /*Final merge*/                                                        
    r8  = __vmrghb(r0, r4);   /*all set 0*/                              
    r9  = __vmrglb(r0, r4);   /*all set 1*/                              
    r10 = __vmrghb(r1, r5);   /*all set 2*/                              
    r11 = __vmrglb(r1, r5);   /*all set 3*/                              
    r12 = __vmrghb(r2, r6);   /*all set 4*/                              
    r13 = __vmrglb(r2, r6);   /*all set 5*/                              
    /* Don't need to compute 14 and 15*/                                                                
}



static void inline transpose4x16(
	register __vector4 r0, 
	register __vector4 r1, 
	register __vector4 r2, 
	register __vector4 r3 
){
	register __vector4 r4;
    register __vector4 r5;
    register __vector4 r6;
    register __vector4 r7;
	
	r4 = __vmrghb(r0, r2);  /*0, 2 set 0*/ \
    r5 = __vmrglb(r0, r2);  /*0, 2 set 1*/ \
    r6 = __vmrghb(r1, r3);  /*1, 3 set 0*/ \
    r7 = __vmrglb(r1, r3);  /*1, 3 set 1*/ \
                                             \
    r0 = __vmrghb(r4, r6);  /*all set 0*/  \
    r1 = __vmrglb(r4, r6);  /*all set 1*/  \
    r2 = __vmrghb(r5, r7);  /*all set 2*/  \
    r3 = __vmrglb(r5, r7);  /*all set 3*/  \
}

static void inline h264_deblock_p0_q0(
	register __vector4 p0, 
	register __vector4 p1, 
	register __vector4 q0, 
	register __vector4 q1, 
	register __vector4 tc0masked
) 
{
                                                                                                  
    const __vector4 A0v = __vslb(__vspltisb(0xA), __vspltisb(4));                               
                                                                                                  
    register __vector4 pq0bit = __vxor(p0,q0);                                                    
    register __vector4 q1minus;                                                                    
    register __vector4 p0minus;                                                                    
    register __vector4 stage1;                                                                     
    register __vector4 stage2;                                                                     
    register __vector4 vec160;                                                                     
    register __vector4 delta;                                                                      
    register __vector4 deltaneg;                                                                   
	
    q1minus = __vnor(q1, q1);                 /* 255 - q1 */                                     
    stage1 = __vavgub(p1, q1minus);             /* (p1 - q1 + 256)>>1 */                           
    stage2 = __vsrb(stage1, __vspltisb(1));  /* (p1 - q1 + 256)>>2 = 64 + (p1 - q1) >> 2 */     
    p0minus = __vnor(p0, p0);                 /* 255 - p0 */                                     
    stage1 = __vavgub(q0, p0minus);             /* (q0 - p0 + 256)>>1 */                           
    pq0bit = __vand(pq0bit, __vspltisb(1));                                                    
    stage2 = __vavgub(stage2, pq0bit);          /* 32 + ((q0 - p0)&1 + (p1 - q1) >> 2 + 1) >> 1 */ 
    stage2 = __vaddubs(stage2, stage1);         /* 160 + ((p0 - q0) + (p1 - q1) >> 2 + 1) >> 1 */  
    vec160 = __lvx(&A0v, 0);                                                                     
    deltaneg = __vsububs(vec160, stage2);       /* -d */
    delta = __vsububs(stage2, vec160);          /* d */
    deltaneg = __vminub(tc0masked, deltaneg);
    delta = __vminub(tc0masked, delta);
    p0 = __vsububs(p0, deltaneg);
    q0 = __vsububs(q0, delta);
    p0 = __vaddubs(p0, delta);
    q0 = __vaddubs(q0, deltaneg);
}

// out: newp1 = clip((p2 + ((p0 + q0 + 1) >> 1)) >> 1, p1-tc0, p1+tc0)
static inline __vector4 h264_deblock_q1(
	register __vector4 p0,
	register __vector4 p1,
	register __vector4 p2,
	register __vector4 q0,
	register __vector4 tc0
   ) 
{

    register __vector4 average = __vavgub(p0, q0);
    register __vector4 temp;
    register __vector4 uncliped;
    register __vector4 ones;
    register __vector4 max;
    register __vector4 min;
    register __vector4 newp1;

    temp = __vxor(average, p2);				//__vnor ?
    average = __vavgub (average, p2);    	/*avg(p2, avg(p0, q0)) */
    ones = __vspltisb(1);					//vspltisb 16,-1
    temp = __vand (temp, ones);        		/*(p2^avg(p0, q0)) & 1 */
    uncliped = __vsububs (average, temp); 	/*(p2+((p0+q0+1)>>1))>>1 */
    max = __vaddubs (p1, tc0);
    min = __vsububs(p1, tc0);
    newp1 = __vmaxub(min, uncliped);
    newp1 = __vminub(max, newp1);
    return newp1;
}

// out: o = |x-y| < a
static inline __vector4 diff_lt_altivec ( 
	__vector4 x,
	__vector4 y,
	__vector4 a
)
{
	__vector4 diff = __vsububs(x, y);
	__vector4 diffneg = __vsububs(y, x);
    __vector4 o = __vor(diff, diffneg); /* |x-y| */
    o = __vcmpgtub(o, a);
    return o;
}

static inline __vector4 h264_deblock_mask ( 
	__vector4 p0,
	__vector4 p1,
	__vector4 q0,
	__vector4 q1,
	__vector4 alpha,
	__vector4 beta
) {

    __vector4 mask;
    __vector4 tempmask;

    mask = diff_lt_altivec(p0, q0, alpha);
    tempmask = diff_lt_altivec(p1, p0, beta);
    mask = __vand(mask, tempmask);
    tempmask = diff_lt_altivec(q1, q0, beta);
    mask = __vand(mask, tempmask);

    return mask;
}

static inline void h264_loop_filter_luma_altivec(
	register __vector4 p2, 
	register __vector4 p1, 
	register __vector4 p0, 
	register __vector4 q0, 
	register __vector4 q1, 
	register __vector4 q2, 
	int alpha, 
	int beta, 
	int8_t * tc0)
{
	DECLARE_ALIGNED(16, unsigned char, temp)[16];
	__vector4 alphavec,betavec,mask,p1mask,q1mask,tc0vec,finaltc0,tc0masked,newp1,newq1;
	
	temp[0] = alpha;
    temp[1] = beta;
	
	alphavec = __lvx(temp,0);
	betavec = __vspltb(alphavec, 0x1);
	alphavec = __vspltb(alphavec, 0x0);
	mask = h264_deblock_mask(p0, p1, q0, q1, alphavec, betavec); /*if in block */
	
	*((int *)temp) = *((int *)tc0);
	tc0vec = __lvx((signed char*)temp, 0);
    tc0vec = __vmrghb(tc0vec, tc0vec);
    tc0vec = __vmrghb(tc0vec, tc0vec);
	mask = __vand(mask, __vcmpgtub(tc0vec, __vspltisb(-1)));//vspltisb 7,1  /* if tc0[i] >= 0 */
	finaltc0 = __vand(tc0vec, mask);
	
	p1mask = diff_lt_altivec(p2, p0, betavec);
	p1mask = __vand(p1mask, mask);
	tc0masked = __vand(p1mask, tc0vec);
	finaltc0 = __vsububm(finaltc0, p1mask);
	
	p1mask = diff_lt_altivec(p2, p0, betavec);
    p1mask = __vand(p1mask, mask);                             /* if ( |p2 - p0| < beta) */ 
    tc0masked = __vand(p1mask, tc0vec);
    finaltc0 = __vsububm(finaltc0, p1mask);                       /* tc++ */                   
    newp1 = h264_deblock_q1(p0, p1, p2, q0, tc0masked);
	
	q1mask = diff_lt_altivec(q2, q0, betavec); 
    q1mask = __vand(q1mask, mask);                             /* if ( |q2 - q0| < beta ) */
    tc0masked = __vand(q1mask, tc0vec);                                           
    finaltc0 = __vsububm(finaltc0, q1mask);                       /* tc++ */                   
    newq1 = h264_deblock_q1(p0, q1, q2, q0, tc0masked);                                 

	h264_deblock_p0_q0(p0, p1, q0, q1, finaltc0);
    p1 = newp1;
    q1 = newq1;     
}

static void h264_v_loop_filter_luma_altivec(uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0) {
    if ((tc0[0] & tc0[1] & tc0[2] & tc0[3]) >= 0) 
	{
        __vector4 p2 = __lvx(pix,-3*stride);
        __vector4 p1 = __lvx(pix,-2*stride);
        __vector4 p0 = __lvx(pix,-1*stride);
        __vector4 q0 = __lvx(pix,0);
        __vector4 q1 = __lvx(pix,stride);
        __vector4 q2 = __lvx(pix,2*stride);
        h264_loop_filter_luma_altivec(p2, p1, p0, q0, q1, q2, alpha, beta, tc0);
        __stvx (p1, pix, -2*stride);
        __stvx (p0, pix, -1*stride);
        __stvx (q0, pix, 0);
        __stvx (q1, pix, stride);
    }
}
/** No finished ** => no speed up ...*/
#if 0
static void h264_h_loop_filter_luma_altivec(uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0) {

    __vector4 line0, line1, line2, line3, line4, line5;
    if ((tc0[0] & tc0[1] & tc0[2] & tc0[3]) < 0)
        return;
    readAndTranspose16x6(pix-3, stride, line0, line1, line2, line3, line4, line5);
    h264_loop_filter_luma_altivec(line0, line1, line2, line3, line4, line5, alpha, beta, tc0);
    transpose4x16(line1, line2, line3, line4);
    write16x4(pix-2, stride, line1, line2, line3, line4);
}
#endif

void ff_biweight_h264_pixels16x16_vmx(uint8_t *dst, uint8_t *src, int stride, int log2_denom, int weightd, int weights, int offset);
void ff_biweight_h264_pixels8x8_vmx(uint8_t *dst, uint8_t *src, int stride, int log2_denom, int weightd, int weights, int offset);

void ff_h264dsp_init_luma_vmx(H264DSPContext *c)
{
	//c->biweight_h264_pixels_tab[0] = ff_biweight_h264_pixels16x16_vmx;
	//c->biweight_h264_pixels_tab[3] = ff_biweight_h264_pixels8x8_vmx;
	c->h264_v_loop_filter_luma= h264_v_loop_filter_luma_altivec;//5Ox more faster !!
	//c->h264_h_loop_filter_luma= h264_h_loop_filter_luma_altivec;
}
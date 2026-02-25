/*
 * Xbox 360 VMX (Vector/SIMD) optimized float DSP functions.
 *
 * Adapted from the working FFmpeg 0.6 Xbox 360 port's
 * libavcodec/xbox360/float_altivec.c (by Luca Barbato / lu_zero@gentoo.org)
 * to the FFmpeg 1.2 AVFloatDSPContext API.
 *
 * Key advantage over C fallbacks: __vmaddfp (fused multiply-add) performs
 * one rounding operation instead of two separate roundings for multiply
 * then add.  This reduces floating-point error accumulation that causes
 * NaN/artifact propagation on the Xbox 360's non-IEEE PPC FPU, especially
 * in the IMDCT overlap-add and SBR QMF synthesis paths.
 *
 * Copyright (c) 2006 Luca Barbato <lu_zero@gentoo.org>
 * Xbox 360 adaptation Copyright (c) 2025
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#ifdef _XBOX

#include <xtl.h>
#include "libavutil/float_dsp.h"

/**
 * vector_fmul: dst[i] = src0[i] * src1[i]
 *
 * Uses __vmaddfp(a, b, zero) which is a single-rounding fused multiply-add.
 * Processes 8 floats (two 128-bit vectors) per iteration.
 * Requirement: len is multiple of 16, pointers 32-byte aligned.
 */
static void vector_fmul_vmx(float *dst, const float *src0,
                             const float *src1, int len)
{
    int i;
    __vector4 d0, d1, s0, s1, zero = __vzero();
    for (i = 0; i < len - 7; i += 8) {
        d0 = __lvx(src0 + i, 0);
        s0 = __lvx(src1 + i, 0);
        d1 = __lvx(src0 + i, 16);
        s1 = __lvx(src1 + i, 16);
        d0 = __vmaddfp(d0, s0, zero);
        d1 = __vmaddfp(d1, s1, zero);
        __stvx(d0, dst + i, 0);
        __stvx(d1, dst + i, 16);
    }
    /* Handle tail elements (if len is not a multiple of 8) */
    for (; i < len; i++)
        dst[i] = src0[i] * src1[i];
}

/**
 * vector_fmul_reverse: dst[i] = src0[i] * src1[len-1-i]
 *
 * Used in SBR QMF analysis (sbr_qmf_analysis).
 * The reverse iteration of src1 is done via merge/permute to reverse
 * each 4-float vector.
 */
static void vector_fmul_reverse_vmx(float *dst, const float *src0,
                                     const float *src1, int len)
{
    int i;
    __vector4 d, s0, s1, h0, l0,
              s2, s3, zero = __vzero();

    src1 += len - 4;
    for (i = 0; i < len - 7; i += 8) {
        s1 = __lvx(src1 - i, 0);              /* [a,b,c,d] */
        s0 = __lvx(src0 + i, 0);
        l0 = __vmrglw(s1, s1);                /* [c,c,d,d] */
        s3 = __lvx(src1 - i, -16);
        h0 = __vmrghw(s1, s1);                /* [a,a,b,b] */
        s2 = __lvx(src0 + i, 16);
        s1 = __vmrghw(__vmrglw(l0, h0),       /* [d,b,d,b] */
                      __vmrghw(l0, h0));       /* [c,a,c,a] */
                                               /* result: [d,c,b,a] */
        l0 = __vmrglw(s3, s3);
        d  = __vmaddfp(s0, s1, zero);
        h0 = __vmrghw(s3, s3);
        __stvx(d, dst + i, 0);
        s3 = __vmrghw(__vmrglw(l0, h0),
                      __vmrghw(l0, h0));
        d  = __vmaddfp(s2, s3, zero);
        __stvx(d, dst + i, 16);
    }
}

/**
 * vector_fmul_add: dst[i] = src0[i] * src1[i] + src2[i]
 *
 * This is the critical function for SBR QMF synthesis — called 10 times
 * per QMF slot.  __vmaddfp(s0, s1, s2) computes the multiply-add in a
 * single operation with one rounding, vs the C version's two operations
 * with two roundings.
 *
 * Uses unaligned load/store via __lvsr/__lvsl/__vperm for dst in case
 * it's not 16-byte aligned (matches the proven 0.6 implementation).
 */
static void vector_fmul_add_vmx(float *dst, const float *src0,
                                 const float *src1, const float *src2,
                                 int len)
{
    int i;
    __vector4 d, s0, s1, s2v, t0, t1, edges;
    __vector4 align, mask;

    align = __lvsr(dst, 0);
    mask  = __lvsl(dst, 0);

    for (i = 0; i < len - 3; i += 4) {
        t0    = __lvx(dst + i, 0);
        t1    = __lvx(dst + i, 15);
        s0    = __lvx(src0 + i, 0);
        s1    = __lvx(src1 + i, 0);
        s2v   = __lvx(src2 + i, 0);
        edges = __vperm(t1, t0, mask);
        d     = __vmaddfp(s0, s1, s2v);
        t1    = __vperm(d, edges, align);
        t0    = __vperm(edges, d, align);
        __stvx(t1, dst + i, 15);
        __stvx(t0, dst + i, 0);
    }
    /* Handle tail */
    for (; i < len; i++)
        dst[i] = src0[i] * src1[i] + src2[i];
}

/**
 * vector_fmul_window: overlap-add with window function.
 *
 * dst[i]     = src0[i] * win[len-1-i] - src1[len-1-i] * win[i]
 * dst[len+j] = src0[j] * win[j]       + src1[len-1-j] * win[len-1-j]
 *   (where j = len-1-i)
 *
 * Used by MDCT-based codecs for overlap-add (imdct_and_windowing).
 * FFmpeg 1.2 API does NOT have the add_bias parameter that 0.6 had.
 *
 * Note: The 0.6 port's float_init_vmx() commented this out
 * (only enabled for non-bit-exact mode).  We enable it here since
 * the Xbox 360 needs the fused multiply-add precision benefit, and
 * with add_bias=0 removed from the API, the only difference from
 * the C version is single vs double rounding.
 */
static void vector_fmul_window_vmx(float *dst, const float *src0,
                                    const float *src1, const float *win,
                                    int len)
{
    int i, j;
    __vector4 zero, t0, t1, s0, s1, wi, wj;

    /*
     * Build a byte-permute vector that reverses four 32-bit words:
     * element 0 gets bytes 12-15, element 1 gets 8-11, etc.
     * On Xbox 360, __vperm interprets the control vector as byte indices.
     */
    static const __declspec(align(16)) unsigned char rev_bytes[16] = {
        0x0C, 0x0D, 0x0E, 0x0F,   /* word 3 -> word 0 */
        0x08, 0x09, 0x0A, 0x0B,   /* word 2 -> word 1 */
        0x04, 0x05, 0x06, 0x07,   /* word 1 -> word 2 */
        0x00, 0x01, 0x02, 0x03    /* word 0 -> word 3 */
    };
    __vector4 reverse = __lvx(rev_bytes, 0);

    dst  += len;
    win  += len;
    src0 += len;

    zero = __vzero();

    for (i = -len * 4, j = len * 4 - 16; i < 0; i += 16, j -= 16) {
        s0 = __lvx(src0, i);
        s1 = __lvx(src1, j);
        wi = __lvx(win,  i);
        wj = __lvx(win,  j);

        s1 = __vperm(s1, s1, reverse);
        wj = __vperm(wj, wj, reverse);

        t0 = __vmaddfp(s0, wj, zero);
        t0 = __vnmsubfp(s1, wi, t0);      /* t0 = s0*wj - s1*wi */
        t1 = __vmaddfp(s0, wi, zero);
        t1 = __vmaddfp(s1, wj, t1);       /* t1 = s0*wi + s1*wj */
        t1 = __vperm(t1, t1, reverse);

        __stvx(t0, dst, i);
        __stvx(t1, dst, j);
    }
}

/**
 * Initialize AVFloatDSPContext with Xbox 360 VMX-optimized functions.
 *
 * Called from avpriv_float_dsp_init() when _XBOX is defined.
 * Replaces the C fallback function pointers with VMX versions that use
 * fused multiply-add instructions for better precision on PPC.
 */
void ff_float_dsp_init_xbox360(AVFloatDSPContext *fdsp)
{
    fdsp->vector_fmul         = vector_fmul_vmx;
    fdsp->vector_fmul_reverse = vector_fmul_reverse_vmx;
    fdsp->vector_fmul_add     = vector_fmul_add_vmx;
    fdsp->vector_fmul_window  = vector_fmul_window_vmx;
}

#endif /* _XBOX */

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

/**
 * @file
 * Xbox 360 VMX DSP functions for FFmpeg 1.2
 *
 * Adapted from FFmpeg 0.6 xbox360/dsputil_ppc.c
 *
 * Changes for FFmpeg 1.2:
 *   - Renamed to ff_dsputil_init_ppc (ff_ prefix)
 *   - DCTELEM replaced with int16_t
 *   - prefetch moved to VideoDSPContext (see videodsp_xbox360.c)
 *   - float ops moved to FmtConvertContext / AVFloatDSPContext
 *   - VC-1 DSP moved to VC1DSPContext (see vc1dsp_xbox360.c)
 *   - H.264 DSP split into H264DSPContext, H264QpelContext, H264ChromaContext
 */

#include "config.h"
#include "libavcodec/dsputil.h"
#include <xtl.h>

/* ***** WARNING *****
 * clear_blocks_dcbz32_ppc will not work properly on PowerPC processors
 * with a cache line size not equal to 32 bytes.
 * Xbox 360 Xenon CPU uses 128-byte cache lines, so dcbz128 path should be used.
 */
static void clear_blocks_dcbz32_ppc(int16_t *blocks)
{
    register int misal = ((unsigned long)blocks & 0x00000010);
    register int i = 0;

    if (misal) {
        ((unsigned long*)blocks)[0] = 0L;
        ((unsigned long*)blocks)[1] = 0L;
        ((unsigned long*)blocks)[2] = 0L;
        ((unsigned long*)blocks)[3] = 0L;
        i += 16;
    }
    for ( ; i < sizeof(int16_t)*6*64-31 ; i += 32) {
        __dcbz(i, blocks);
    }
    if (misal) {
        ((unsigned long*)blocks)[188] = 0L;
        ((unsigned long*)blocks)[189] = 0L;
        ((unsigned long*)blocks)[190] = 0L;
        ((unsigned long*)blocks)[191] = 0L;
        i += 16;
    }
}

/* Same as above, when dcbzl clears a whole 128B cache line
 * i.e. the Xbox 360 Xenon CPU */
static void clear_blocks_dcbz128_ppc(int16_t *blocks)
{
    register int misal = ((unsigned long)blocks & 0x0000007f);
    register int i = 0;

    if (misal) {
        XMemSet(blocks, 0, sizeof(int16_t)*6*64);
    }
    else {
        for ( ; i < sizeof(int16_t)*6*64 ; i += 128) {
            __dcbz128(i, blocks);
        }
    }
}

/* Check dcbz report how many bytes are set to 0 by dcbz */
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

    __dcbz128(i, fakedata_middle);

    for (i = 0; i < 1024 ; i ++) {
        if (fakedata[i] == (char)0)
            count++;
    }

    av_free(fakedata);
    return count;
}

static void clear_block_vmx(int16_t *block)
{
    __stvx(__vzero(), block,   0);
    __stvx(__vzero(), block,  16);
    __stvx(__vzero(), block,  32);
    __stvx(__vzero(), block,  48);
    __stvx(__vzero(), block,  64);
    __stvx(__vzero(), block,  80);
    __stvx(__vzero(), block,  96);
    __stvx(__vzero(), block, 112);
}

av_cold void ff_dsputil_init_ppc(DSPContext *c, AVCodecContext *avctx)
{
    const int high_bit_depth = avctx->bits_per_raw_sample > 8;

    if (high_bit_depth) {
        /* No VMX optimization for high bit depth yet */
        return;
    }

    /* Cache-line zeroing for block clearing */
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
}

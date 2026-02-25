/*
 * Xbox 360 VMX VideoDSP functions for FFmpeg 1.2
 *
 * In FFmpeg 1.2, the prefetch function moved from DSPContext
 * into a new VideoDSPContext. This file provides the Xbox 360
 * implementation.
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

#include "config.h"
#include "libavcodec/videodsp.h"
#include <xtl.h>

static void prefetch_ppc(void *mem, int stride, int h)
{
    register const uint8_t *p = mem;
    do {
        __dcbt(0, p);
        p += stride;
    } while (--h);
}

av_cold void ff_videodsp_init_ppc(VideoDSPContext *ctx, int bpc)
{
    ctx->prefetch = prefetch_ppc;
}

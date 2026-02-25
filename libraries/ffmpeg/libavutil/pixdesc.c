/*
 * pixel format descriptor
 * Copyright (c) 2009 Michael Niedermayer <michaelni@gmx.at>
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

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "pixfmt.h"
#include "pixdesc.h"

#include "intreadwrite.h"

void av_read_image_line(uint16_t *dst,
                        const uint8_t *data[4], const int linesize[4],
                        const AVPixFmtDescriptor *desc,
                        int x, int y, int c, int w,
                        int read_pal_component)
{
    AVComponentDescriptor comp = desc->comp[c];
    int plane = comp.plane;
    int depth = comp.depth_minus1 + 1;
    int mask  = (1 << depth) - 1;
    int shift = comp.shift;
    int step  = comp.step_minus1 + 1;
    int flags = desc->flags;

    if (flags & PIX_FMT_BITSTREAM) {
        int skip = x * step + comp.offset_plus1 - 1;
        const uint8_t *p = data[plane] + y * linesize[plane] + (skip >> 3);
        int shift = 8 - depth - (skip & 7);

        while (w--) {
            int val = (*p >> shift) & mask;
            if (read_pal_component)
                val = data[1][4*val + c];
            shift -= step;
            p -= shift >> 3;
            shift &= 7;
            *dst++ = val;
        }
    } else {
        const uint8_t *p = data[plane] + y * linesize[plane] +
                           x * step + comp.offset_plus1 - 1;
        int is_8bit = shift + depth <= 8;

        if (is_8bit)
            p += !!(flags & PIX_FMT_BE);

        while (w--) {
            int val = is_8bit ? *p :
                flags & PIX_FMT_BE ? AV_RB16(p) : AV_RL16(p);
            val = (val >> shift) & mask;
            if (read_pal_component)
                val = data[1][4 * val + c];
            p += step;
            *dst++ = val;
        }
    }
}

void av_write_image_line(const uint16_t *src,
                         uint8_t *data[4], const int linesize[4],
                         const AVPixFmtDescriptor *desc,
                         int x, int y, int c, int w)
{
    AVComponentDescriptor comp = desc->comp[c];
    int plane = comp.plane;
    int depth = comp.depth_minus1 + 1;
    int step  = comp.step_minus1 + 1;
    int flags = desc->flags;

    if (flags & PIX_FMT_BITSTREAM) {
        int skip = x * step + comp.offset_plus1 - 1;
        uint8_t *p = data[plane] + y * linesize[plane] + (skip >> 3);
        int shift = 8 - depth - (skip & 7);

        while (w--) {
            *p |= *src++ << shift;
            shift -= step;
            p -= shift >> 3;
            shift &= 7;
        }
    } else {
        int shift = comp.shift;
        uint8_t *p = data[plane] + y * linesize[plane] +
                     x * step + comp.offset_plus1 - 1;

        if (shift + depth <= 8) {
            p += !!(flags & PIX_FMT_BE);
            while (w--) {
                *p |= (*src++ << shift);
                p += step;
            }
        } else {
            while (w--) {
                if (flags & PIX_FMT_BE) {
                    uint16_t val = AV_RB16(p) | (*src++ << shift);
                    AV_WB16(p, val);
                } else {
                    uint16_t val = AV_RL16(p) | (*src++ << shift);
                    AV_WL16(p, val);
                }
                p += step;
            }
        }
    }
}

#if !FF_API_PIX_FMT_DESC
static
#endif
const AVPixFmtDescriptor av_pix_fmt_descriptors[AV_PIX_FMT_NB] = {
    {
        "yuv420p", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuyv422", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        0, /* flags */
        {
            { 0, 1, 1, 0, 7 },        /* Y */
            { 0, 3, 2, 0, 7 },        /* U */
            { 0, 3, 4, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "rgb24", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 2, 1, 0, 7 },        /* R */
            { 0, 2, 2, 0, 7 },        /* G */
            { 0, 2, 3, 0, 7 },        /* B */
        }, /* comp */
    },
    {
        "bgr24", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 2, 3, 0, 7 },        /* R */
            { 0, 2, 2, 0, 7 },        /* G */
            { 0, 2, 1, 0, 7 },        /* B */
        }, /* comp */
    },
    {
        "yuv422p", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuv410p", /* name */
        3, /* nb_components */
        2, /* log2_chroma_w */
        2, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuv411p", /* name */
        3, /* nb_components */
        2, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "gray", /* name */
        1, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PSEUDOPAL, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
        }, /* comp */
    },
    {
        "monow", /* name */
        1, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BITSTREAM, /* flags */
        {
            { 0, 0, 1, 0, 0 },        /* Y */
        }, /* comp */
    },
    {
        "monob", /* name */
        1, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BITSTREAM, /* flags */
        {
            { 0, 0, 1, 7, 0 },        /* Y */
        }, /* comp */
    },
    {
        "pal8", /* name */
        1, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PAL, /* flags */
        {
            { 0, 0, 1, 0, 7 },
        }, /* comp */
    },
    {
        "yuvj420p", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuvj422p", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuvj444p", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "xvmcmc", /* name */
        0, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "xvmcidct", /* name */
        0, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "uyvy422", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        0, /* flags */
        {
            { 0, 1, 2, 0, 7 },        /* Y */
            { 0, 3, 1, 0, 7 },        /* U */
            { 0, 3, 3, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "uyyvyy411", /* name */
        3, /* nb_components */
        2, /* log2_chroma_w */
        0, /* log2_chroma_h */
        0, /* flags */
        {
            { 0, 3, 2, 0, 7 },        /* Y */
            { 0, 5, 1, 0, 7 },        /* U */
            { 0, 5, 4, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "bgr8", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_PSEUDOPAL, /* flags */
        {
            { 0, 0, 1, 0, 2 },        /* R */
            { 0, 0, 1, 3, 2 },        /* G */
            { 0, 0, 1, 6, 1 },        /* B */
        }, /* comp */
    },
    {
        "bgr4", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BITSTREAM | PIX_FMT_RGB, /* flags */
        {
            { 0, 3, 4, 0, 0 },        /* R */
            { 0, 3, 2, 0, 1 },        /* G */
            { 0, 3, 1, 0, 0 },        /* B */
        }, /* comp */
    },
    {
        "bgr4_byte", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_PSEUDOPAL, /* flags */
        {
            { 0, 0, 1, 0, 0 },        /* R */
            { 0, 0, 1, 1, 1 },        /* G */
            { 0, 0, 1, 3, 0 },        /* B */
        }, /* comp */
    },
    {
        "rgb8", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_PSEUDOPAL, /* flags */
        {
            { 0, 0, 1, 6, 1 },        /* R */
            { 0, 0, 1, 3, 2 },        /* G */
            { 0, 0, 1, 0, 2 },        /* B */
        }, /* comp */
    },
    {
        "rgb4", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BITSTREAM | PIX_FMT_RGB, /* flags */
        {
            { 0, 3, 1, 0, 0 },        /* R */
            { 0, 3, 2, 0, 1 },        /* G */
            { 0, 3, 4, 0, 0 },        /* B */
        }, /* comp */
    },
    {
        "rgb4_byte", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_PSEUDOPAL, /* flags */
        {
            { 0, 0, 1, 3, 0 },        /* R */
            { 0, 0, 1, 1, 1 },        /* G */
            { 0, 0, 1, 0, 0 },        /* B */
        }, /* comp */
    },
    {
        "nv12", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 1, 1, 0, 7 },        /* U */
            { 1, 1, 2, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "nv21", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 1, 2, 0, 7 },        /* U */
            { 1, 1, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "argb", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 3, 2, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 4, 0, 7 },        /* B */
            { 0, 3, 1, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "rgba", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 3, 1, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 3, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "abgr", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 3, 4, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 2, 0, 7 },        /* B */
            { 0, 3, 1, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "bgra", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 3, 3, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 1, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "0rgb", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 3, 2, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 4, 0, 7 },        /* B */
        }, /* comp */
    },
    {
        "rgb0", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 3, 1, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 3, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "0bgr", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 3, 4, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 2, 0, 7 },        /* B */
        }, /* comp */
    },
    {
        "bgr0", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 3, 3, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 1, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "gray16be", /* name */
        1, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE, /* flags */
        {
            { 0, 1, 1, 0, 15 },       /* Y */
        }, /* comp */
    },
    {
        "gray16le", /* name */
        1, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        0, /* flags */
        {
            { 0, 1, 1, 0, 15 },       /* Y */
        }, /* comp */
    },
    {
        "yuv440p", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuvj440p", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        }, /* comp */
    },
    {
        "yuva420p", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
            { 3, 0, 1, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "yuva422p", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
            { 3, 0, 1, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "yuva444p", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
            { 3, 0, 1, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "yuva420p9be", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
            { 3, 1, 1, 0, 8 },        /* A */
        }, /* comp */
    },
    {
        "yuva420p9le", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
            { 3, 1, 1, 0, 8 },        /* A */
        }, /* comp */
    },
    {
        "yuva422p9be", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
            { 3, 1, 1, 0, 8 },        /* A */
        }, /* comp */
    },
    {
        "yuva422p9le", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
            { 3, 1, 1, 0, 8 },        /* A */
        }, /* comp */
    },
    {
        "yuva444p9be", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
            { 3, 1, 1, 0, 8 },        /* A */
        }, /* comp */
    },
    {
        "yuva444p9le", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
            { 3, 1, 1, 0, 8 },        /* A */
        }, /* comp */
    },
    {
        "yuva420p10be", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
            { 3, 1, 1, 0, 9 },        /* A */
        }, /* comp */
    },
    {
        "yuva420p10le", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
            { 3, 1, 1, 0, 9 },        /* A */
        }, /* comp */
    },
    {
        "yuva422p10be", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
            { 3, 1, 1, 0, 9 },        /* A */
        }, /* comp */
    },
    {
        "yuva422p10le", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
            { 3, 1, 1, 0, 9 },        /* A */
        }, /* comp */
    },
    {
        "yuva444p10be", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
            { 3, 1, 1, 0, 9 },        /* A */
        }, /* comp */
    },
    {
        "yuva444p10le", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
            { 3, 1, 1, 0, 9 },        /* A */
        }, /* comp */
    },
    {
        "yuva420p16be", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
            { 3, 1, 1, 0, 15 },        /* A */
        }, /* comp */
    },
    {
        "yuva420p16le", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
            { 3, 1, 1, 0, 15 },        /* A */
        }, /* comp */
    },
    {
        "yuva422p16be", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
            { 3, 1, 1, 0, 15 },        /* A */
        }, /* comp */
    },
    {
        "yuva422p16le", /* name */
        4, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
            { 3, 1, 1, 0, 15 },        /* A */
        }, /* comp */
    },
    {
        "yuva444p16be", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
            { 3, 1, 1, 0, 15 },        /* A */
        }, /* comp */
    },
    {
        "yuva444p16le", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
            { 3, 1, 1, 0, 15 },        /* A */
        }, /* comp */
    },
    {
        "vdpau_h264", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vdpau_mpeg1", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vdpau_mpeg2", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vdpau_wmv3", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vdpau_vc1", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vdpau_mpeg4", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "rgb48be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_BE, /* flags */
        {
            { 0, 5, 1, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 5, 0, 15 },       /* B */
        }, /* comp */
    },
    {
        "rgb48le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 5, 1, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 5, 0, 15 },       /* B */
        }, /* comp */
    },
    {
        "rgba64be", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_BE | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 7, 1, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 5, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        }, /* comp */
    },
    {
        "rgba64le", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 7, 1, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 5, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        }, /* comp */
    },
    {
        "rgb565be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 0, 3, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        }, /* comp */
    },
    {
        "rgb565le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 2, 3, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        }, /* comp */
    },
    {
        "rgb555be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 0, 2, 4 },        /* R */
            { 0, 1, 1, 5, 4 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        }, /* comp */
    },
    {
        "rgb555le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 2, 2, 4 },        /* R */
            { 0, 1, 1, 5, 4 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        }, /* comp */
    },
    {
        "rgb444be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 0, 0, 3 },        /* R */
            { 0, 1, 1, 4, 3 },        /* G */
            { 0, 1, 1, 0, 3 },        /* B */
        }, /* comp */
    },
    {
        "rgb444le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 2, 0, 3 },        /* R */
            { 0, 1, 1, 4, 3 },        /* G */
            { 0, 1, 1, 0, 3 },        /* B */
        }, /* comp */
    },
    {
        "bgr48be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB, /* flags */
        {
            { 0, 5, 5, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 1, 0, 15 },       /* B */
        }, /* comp */
    },
    {
        "bgr48le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 5, 5, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 1, 0, 15 },       /* B */
        }, /* comp */
    },
    {
        "bgra64be", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 7, 5, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 1, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        }, /* comp */
    },
    {
        "bgra64le", /* name */
        4, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB | PIX_FMT_ALPHA, /* flags */
        {
            { 0, 7, 5, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 1, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        }, /* comp */
    },
    {
        "bgr565be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 1, 0, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 0, 3, 4 },        /* B */
        }, /* comp */
    },
    {
        "bgr565le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 1, 0, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 2, 3, 4 },        /* B */
        }, /* comp */
    },
    {
        "bgr555be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 1, 0, 4 },       /* R */
            { 0, 1, 1, 5, 4 },       /* G */
            { 0, 1, 0, 2, 4 },       /* B */
        }, /* comp */
    },
    {
        "bgr555le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 1, 0, 4 },        /* R */
            { 0, 1, 1, 5, 4 },        /* G */
            { 0, 1, 2, 2, 4 },        /* B */
        }, /* comp */
    },
    {
        "bgr444be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 1, 0, 3 },       /* R */
            { 0, 1, 1, 4, 3 },       /* G */
            { 0, 1, 0, 0, 3 },       /* B */
        }, /* comp */
    },
    {
        "bgr444le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_RGB, /* flags */
        {
            { 0, 1, 1, 0, 3 },        /* R */
            { 0, 1, 1, 4, 3 },        /* G */
            { 0, 1, 2, 0, 3 },        /* B */
        }, /* comp */
    },
    {
        "vaapi_moco", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vaapi_idct", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vaapi_vld", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "yuv420p9le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p9be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p10le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p10be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p12le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p12be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p14le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p14be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p16le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        }, /* comp */
    },
    {
        "yuv420p16be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p9le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p9be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p10le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p10be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p12le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p12be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p14le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p14be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p16le", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        }, /* comp */
    },
    {
        "yuv422p16be", /* name */
        3, /* nb_components */
        1, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p16le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p16be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p10le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p10be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p9le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p9be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p12le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p12be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p14le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        }, /* comp */
    },
    {
        "yuv444p14be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR, /* flags */
        {
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        }, /* comp */
    },
    {
        "dxva2_vld", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "vda_vld", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
    {
        "gray8a", /* name */
        2, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_ALPHA, /* flags */
        {
            { 0, 1, 1, 0, 7 },        /* Y */
            { 0, 1, 2, 0, 7 },        /* A */
        }, /* comp */
    },
    {
        "gbrp", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 0, 1, 0, 7 },        /* R */
            { 0, 0, 1, 0, 7 },        /* G */
            { 1, 0, 1, 0, 7 },        /* B */
        }, /* comp */
    },
    {
        "gbrp9le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 8 },        /* R */
            { 0, 1, 1, 0, 8 },        /* G */
            { 1, 1, 1, 0, 8 },        /* B */
        }, /* comp */
    },
    {
        "gbrp9be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 8 },        /* R */
            { 0, 1, 1, 0, 8 },        /* G */
            { 1, 1, 1, 0, 8 },        /* B */
        }, /* comp */
    },
    {
        "gbrp10le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 9 },        /* R */
            { 0, 1, 1, 0, 9 },        /* G */
            { 1, 1, 1, 0, 9 },        /* B */
        }, /* comp */
    },
    {
        "gbrp10be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 9 },        /* R */
            { 0, 1, 1, 0, 9 },        /* G */
            { 1, 1, 1, 0, 9 },        /* B */
        }, /* comp */
    },
    {
        "gbrp12le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 11 },        /* R */
            { 0, 1, 1, 0, 11 },        /* G */
            { 1, 1, 1, 0, 11 },        /* B */
        }, /* comp */
    },
    {
        "gbrp12be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 11 },        /* R */
            { 0, 1, 1, 0, 11 },        /* G */
            { 1, 1, 1, 0, 11 },        /* B */
        }, /* comp */
    },
    {
        "gbrp14le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 13 },        /* R */
            { 0, 1, 1, 0, 13 },        /* G */
            { 1, 1, 1, 0, 13 },        /* B */
        }, /* comp */
    },
    {
        "gbrp14be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 13 },        /* R */
            { 0, 1, 1, 0, 13 },        /* G */
            { 1, 1, 1, 0, 13 },        /* B */
        }, /* comp */
    },
    {
        "gbrp16le", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 15 },       /* R */
            { 0, 1, 1, 0, 15 },       /* G */
            { 1, 1, 1, 0, 15 },       /* B */
        }, /* comp */
    },
    {
        "gbrp16be", /* name */
        3, /* nb_components */
        0, /* log2_chroma_w */
        0, /* log2_chroma_h */
        PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB, /* flags */
        {
            { 2, 1, 1, 0, 15 },       /* R */
            { 0, 1, 1, 0, 15 },       /* G */
            { 1, 1, 1, 0, 15 },       /* B */
        }, /* comp */
    },
    {
        "vdpau", /* name */
        0, /* nb_components */
        1, /* log2_chroma_w */
        1, /* log2_chroma_h */
        PIX_FMT_HWACCEL, /* flags */
    },
};

static enum AVPixelFormat get_pix_fmt_internal(const char *name)
{
    enum AVPixelFormat pix_fmt;

    for (pix_fmt = 0; pix_fmt < AV_PIX_FMT_NB; pix_fmt++)
        if (av_pix_fmt_descriptors[pix_fmt].name &&
            !strcmp(av_pix_fmt_descriptors[pix_fmt].name, name))
            return pix_fmt;

    return AV_PIX_FMT_NONE;
}

const char *av_get_pix_fmt_name(enum AVPixelFormat pix_fmt)
{
    return (unsigned)pix_fmt < AV_PIX_FMT_NB ?
        av_pix_fmt_descriptors[pix_fmt].name : NULL;
}

#if HAVE_BIGENDIAN
#   define X_NE(be, le) be
#else
#   define X_NE(be, le) le
#endif

enum AVPixelFormat av_get_pix_fmt(const char *name)
{
    enum AVPixelFormat pix_fmt;

    if (!strcmp(name, "rgb32"))
        name = X_NE("argb", "bgra");
    else if (!strcmp(name, "bgr32"))
        name = X_NE("abgr", "rgba");

    pix_fmt = get_pix_fmt_internal(name);
    if (pix_fmt == AV_PIX_FMT_NONE) {
        char name2[32];

        snprintf(name2, sizeof(name2), "%s%s", name, X_NE("be", "le"));
        pix_fmt = get_pix_fmt_internal(name2);
    }
    return pix_fmt;
}

int av_get_bits_per_pixel(const AVPixFmtDescriptor *pixdesc)
{
    int c, bits = 0;
    int log2_pixels = pixdesc->log2_chroma_w + pixdesc->log2_chroma_h;

    for (c = 0; c < pixdesc->nb_components; c++) {
        int s = c == 1 || c == 2 ? 0 : log2_pixels;
        bits += (pixdesc->comp[c].depth_minus1 + 1) << s;
    }

    return bits >> log2_pixels;
}

int av_get_padded_bits_per_pixel(const AVPixFmtDescriptor *pixdesc)
{
    int c, bits = 0;
    int log2_pixels = pixdesc->log2_chroma_w + pixdesc->log2_chroma_h;
    int steps[4] = {0};

    for (c = 0; c < pixdesc->nb_components; c++) {
        const AVComponentDescriptor *comp = &pixdesc->comp[c];
        int s = c == 1 || c == 2 ? 0 : log2_pixels;
        steps[comp->plane] = (comp->step_minus1 + 1) << s;
    }
    for (c = 0; c < 4; c++)
        bits += steps[c];

    if(!(pixdesc->flags & PIX_FMT_BITSTREAM))
        bits *= 8;

    return bits >> log2_pixels;
}

char *av_get_pix_fmt_string (char *buf, int buf_size, enum AVPixelFormat pix_fmt)
{
    /* print header */
    if (pix_fmt < 0) {
       snprintf (buf, buf_size, "name" " nb_components" " nb_bits");
    } else {
        const AVPixFmtDescriptor *pixdesc = &av_pix_fmt_descriptors[pix_fmt];
        snprintf(buf, buf_size, "%-11s %7d %10d", pixdesc->name,
                 pixdesc->nb_components, av_get_bits_per_pixel(pixdesc));
    }

    return buf;
}

const AVPixFmtDescriptor *av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt)
{
    if (pix_fmt < 0 || pix_fmt >= AV_PIX_FMT_NB)
        return NULL;
    return &av_pix_fmt_descriptors[pix_fmt];
}

const AVPixFmtDescriptor *av_pix_fmt_desc_next(const AVPixFmtDescriptor *prev)
{
    if (!prev)
        return &av_pix_fmt_descriptors[0];
    while (prev - av_pix_fmt_descriptors < FF_ARRAY_ELEMS(av_pix_fmt_descriptors) - 1) {
        prev++;
        if (prev->name)
            return prev;
    }
    return NULL;
}

enum AVPixelFormat av_pix_fmt_desc_get_id(const AVPixFmtDescriptor *desc)
{
    if (desc < av_pix_fmt_descriptors ||
        desc >= av_pix_fmt_descriptors + FF_ARRAY_ELEMS(av_pix_fmt_descriptors))
        return AV_PIX_FMT_NONE;

    return desc - av_pix_fmt_descriptors;
}

int av_pix_fmt_get_chroma_sub_sample(enum AVPixelFormat pix_fmt,
                                     int *h_shift, int *v_shift)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
    if (!desc)
        return AVERROR(ENOSYS);
    *h_shift = desc->log2_chroma_w;
    *v_shift = desc->log2_chroma_h;

    return 0;
}

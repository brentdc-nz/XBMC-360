/*
 * PNM image format
 * Copyright (c) 2002, 2003 Fabrice Bellard
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

#include "libavutil/pixdesc.h"
#include "avcodec.h"
#include "internal.h"
#include "pnm.h"


static int pnm_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    PNMContext *s     = avctx->priv_data;
    AVFrame * const p = &s->picture;
    int i, h, h1, c, n, linesize, ret;
    uint8_t *ptr, *ptr1, *ptr2;

    if ((ret = ff_alloc_packet2(avctx, pkt, avpicture_get_size(avctx->pix_fmt,
                                                       avctx->width,
                                                       avctx->height) + 200)) < 0)
        return ret;

    *p           = *pict;
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;

    s->bytestream_start =
    s->bytestream       = pkt->data;
    s->bytestream_end   = pkt->data + pkt->size;

    h  = avctx->height;
    h1 = h;
    switch (avctx->pix_fmt) {
    case AV_PIX_FMT_MONOWHITE:
        c  = '4';
        n  = (avctx->width + 7) >> 3;
        break;
    case AV_PIX_FMT_GRAY8:
        c  = '5';
        n  = avctx->width;
        break;
    case AV_PIX_FMT_GRAY16BE:
        c  = '5';
        n  = avctx->width * 2;
        break;
    case AV_PIX_FMT_RGB24:
        c  = '6';
        n  = avctx->width * 3;
        break;
    case AV_PIX_FMT_RGB48BE:
        c  = '6';
        n  = avctx->width * 6;
        break;
    case AV_PIX_FMT_YUV420P:
        if (avctx->width & 1 || avctx->height & 1) {
            av_log(avctx, AV_LOG_ERROR, "pgmyuv needs even width and height\n");
            return AVERROR(EINVAL);
        }
        c  = '5';
        n  = avctx->width;
        h1 = (h * 3) / 2;
        break;
    case AV_PIX_FMT_YUV420P16BE:
        c  = '5';
        n  = avctx->width * 2;
        h1 = (h * 3) / 2;
        break;
    default:
        return -1;
    }
    snprintf(s->bytestream, s->bytestream_end - s->bytestream,
             "P%c\n%d %d\n", c, avctx->width, h1);
    s->bytestream += strlen(s->bytestream);
    if (avctx->pix_fmt != AV_PIX_FMT_MONOWHITE) {
        int maxdepth = (1 << (av_pix_fmt_desc_get(avctx->pix_fmt)->comp[0].depth_minus1 + 1)) - 1;
        snprintf(s->bytestream, s->bytestream_end - s->bytestream,
                 "%d\n", maxdepth);
        s->bytestream += strlen(s->bytestream);
    }

    ptr      = p->data[0];
    linesize = p->linesize[0];
    for (i = 0; i < h; i++) {
        memcpy(s->bytestream, ptr, n);
        s->bytestream += n;
        ptr           += linesize;
    }

    if (avctx->pix_fmt == AV_PIX_FMT_YUV420P || avctx->pix_fmt == AV_PIX_FMT_YUV420P16BE) {
        h >>= 1;
        n >>= 1;
        ptr1 = p->data[1];
        ptr2 = p->data[2];
        for (i = 0; i < h; i++) {
            memcpy(s->bytestream, ptr1, n);
            s->bytestream += n;
            memcpy(s->bytestream, ptr2, n);
            s->bytestream += n;
                ptr1 += p->linesize[1];
                ptr2 += p->linesize[2];
        }
    }
    pkt->size   = s->bytestream - s->bytestream_start;
    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;

    return 0;
}


#if CONFIG_PGM_ENCODER
static const enum AVPixelFormat _ff_pnmenc_fmts_125[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_GRAY16BE, AV_PIX_FMT_NONE };
AVCodec ff_pgm_encoder = {
        "pgm", /* name */
        NULL_IF_CONFIG_SMALL("PGM (Portable GrayMap) image"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_PGM, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_pnmenc_fmts_125, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(PNMContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_pnm_init, /* init */
        0, /* encode_sub */
        pnm_encode_frame, /* encode2 */
    };
#endif

#if CONFIG_PGMYUV_ENCODER
static const enum AVPixelFormat _ff_pnmenc_fmts_126[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV420P16BE, AV_PIX_FMT_NONE };
AVCodec ff_pgmyuv_encoder = {
        "pgmyuv", /* name */
        NULL_IF_CONFIG_SMALL("PGMYUV (Portable GrayMap YUV) image"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_PGMYUV, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_pnmenc_fmts_126, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(PNMContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_pnm_init, /* init */
        0, /* encode_sub */
        pnm_encode_frame, /* encode2 */
    };
#endif

#if CONFIG_PPM_ENCODER
static const enum AVPixelFormat _ff_pnmenc_fmts_127[] = { AV_PIX_FMT_RGB24, AV_PIX_FMT_RGB48BE, AV_PIX_FMT_NONE };
AVCodec ff_ppm_encoder = {
        "ppm", /* name */
        NULL_IF_CONFIG_SMALL("PPM (Portable PixelMap) image"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_PPM, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_pnmenc_fmts_127, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(PNMContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_pnm_init, /* init */
        0, /* encode_sub */
        pnm_encode_frame, /* encode2 */
    };
#endif

#if CONFIG_PBM_ENCODER
static const enum AVPixelFormat _ff_pnmenc_fmts_128[] = { AV_PIX_FMT_MONOWHITE,
                                                  AV_PIX_FMT_NONE };
AVCodec ff_pbm_encoder = {
        "pbm", /* name */
        NULL_IF_CONFIG_SMALL("PBM (Portable BitMap) image"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_PBM, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_pnmenc_fmts_128, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(PNMContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_pnm_init, /* init */
        0, /* encode_sub */
        pnm_encode_frame, /* encode2 */
    };
#endif

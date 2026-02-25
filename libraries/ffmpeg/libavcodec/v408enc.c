/*
 * v408 encoder
 *
 * Copyright (c) 2012 Carl Eugen Hoyos
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

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"

static av_cold int v408_encode_init(AVCodecContext *avctx)
{
    avctx->coded_frame = avcodec_alloc_frame();

    if (!avctx->coded_frame) {
        av_log(avctx, AV_LOG_ERROR, "Could not allocate frame.\n");
        return AVERROR(ENOMEM);
    }

    return 0;
}

static int v408_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                             const AVFrame *pic, int *got_packet)
{
    uint8_t *dst;
    uint8_t *y, *u, *v, *a;
    int i, j, ret;

    if ((ret = ff_alloc_packet2(avctx, pkt, avctx->width * avctx->height * 4)) < 0)
        return ret;
    dst = pkt->data;

    avctx->coded_frame->reference = 0;
    avctx->coded_frame->key_frame = 1;
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;

    y = pic->data[0];
    u = pic->data[1];
    v = pic->data[2];
    a = pic->data[3];

    for (i = 0; i < avctx->height; i++) {
        for (j = 0; j < avctx->width; j++) {
           if (avctx->codec_id==AV_CODEC_ID_AYUV) {
                *dst++ = v[j];
                *dst++ = u[j];
                *dst++ = y[j];
                *dst++ = a[j];
            } else {
                *dst++ = u[j];
                *dst++ = y[j];
                *dst++ = v[j];
                *dst++ = a[j];
            }
        }
        y += pic->linesize[0];
        u += pic->linesize[1];
        v += pic->linesize[2];
        a += pic->linesize[3];
    }

    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;
    return 0;
}

static av_cold int v408_encode_close(AVCodecContext *avctx)
{
    av_freep(&avctx->coded_frame);

    return 0;
}

#if CONFIG_AYUV_ENCODER
static const enum AVPixelFormat _ff_v408enc_fmts_156[] = { AV_PIX_FMT_YUVA444P, AV_PIX_FMT_NONE };
AVCodec ff_ayuv_encoder = {
        "ayuv", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed packed MS 4:4:4:4"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_AYUV, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_v408enc_fmts_156, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        0, /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        v408_encode_init, /* init */
        0, /* encode_sub */
        v408_encode_frame, /* encode2 */
        0, /* decode */
        v408_encode_close, /* close */
    };
#endif
#if CONFIG_V408_ENCODER
static const enum AVPixelFormat _ff_v408enc_fmts_157[] = { AV_PIX_FMT_YUVA444P, AV_PIX_FMT_NONE };
AVCodec ff_v408_encoder = {
        "v408", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed packed QT 4:4:4:4"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_V408, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_v408enc_fmts_157, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        0, /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        v408_encode_init, /* init */
        0, /* encode_sub */
        v408_encode_frame, /* encode2 */
        0, /* decode */
        v408_encode_close, /* close */
    };
#endif

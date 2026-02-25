/*
 * Copyright (C) 2009 Michael Niedermayer <michaelni@gmx.at>
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

#include "avcodec.h"
#include "internal.h"
#include "libavutil/bswap.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

static av_cold int decode_init(AVCodecContext *avctx)
{
    if (avctx->width & 1) {
        av_log(avctx, AV_LOG_ERROR, "v210x needs even width\n");
        return AVERROR(EINVAL);
    }
    avctx->pix_fmt             = AV_PIX_FMT_YUV422P16;
    avctx->bits_per_raw_sample = 10;

    avctx->coded_frame= avcodec_alloc_frame();
    if (!avctx->coded_frame)
        return AVERROR(ENOMEM);

    return 0;
}

static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
                        AVPacket *avpkt)
{
    const uint32_t *src = (const uint32_t *)avpkt->data;
    AVFrame *pic        = avctx->coded_frame;
    int width           = avctx->width;
    int y               = 0;
    uint16_t *ydst, *udst, *vdst, *yend;
    int ret;

    if (pic->data[0])
        avctx->release_buffer(avctx, pic);

    if (avpkt->size < avctx->width * avctx->height * 8 / 3) {
        av_log(avctx, AV_LOG_ERROR, "Packet too small\n");
        return AVERROR_INVALIDDATA;
    }

    if (avpkt->size > avctx->width * avctx->height * 8 / 3) {
        av_log_ask_for_sample(avctx, "Probably padded data\n");
    }

    pic->reference = 0;
    if ((ret = ff_get_buffer(avctx, pic)) < 0)
        return ret;

    ydst = (uint16_t *)pic->data[0];
    udst = (uint16_t *)pic->data[1];
    vdst = (uint16_t *)pic->data[2];
    yend = ydst + width;
    pic->pict_type = AV_PICTURE_TYPE_I;
    pic->key_frame = 1;

    for (;;) {
        uint32_t v = av_be2ne32(*src++);
        *udst++ = (v >> 16) & 0xFFC0;
        *ydst++ = (v >> 6 ) & 0xFFC0;
        *vdst++ = (v << 4 ) & 0xFFC0;

        v       = av_be2ne32(*src++);
        *ydst++ = (v >> 16) & 0xFFC0;

        if (ydst >= yend) {
            ydst += pic->linesize[0] / 2 - width;
            udst += pic->linesize[1] / 2 - width / 2;
            vdst += pic->linesize[2] / 2 - width / 2;
            yend = ydst + width;
            if (++y >= avctx->height)
                break;
        }

        *udst++ = (v >> 6 ) & 0xFFC0;
        *ydst++ = (v << 4 ) & 0xFFC0;

        v = av_be2ne32(*src++);
        *vdst++ = (v >> 16) & 0xFFC0;
        *ydst++ = (v >> 6 ) & 0xFFC0;

        if (ydst >= yend) {
            ydst += pic->linesize[0] / 2 - width;
            udst += pic->linesize[1] / 2 - width / 2;
            vdst += pic->linesize[2] / 2 - width / 2;
            yend  = ydst + width;
            if (++y >= avctx->height)
                break;
        }

        *udst++ = (v << 4 ) & 0xFFC0;

        v = av_be2ne32(*src++);
        *ydst++ = (v >> 16) & 0xFFC0;
        *vdst++ = (v >> 6 ) & 0xFFC0;
        *ydst++ = (v << 4 ) & 0xFFC0;
        if (ydst >= yend) {
            ydst += pic->linesize[0] / 2 - width;
            udst += pic->linesize[1] / 2 - width / 2;
            vdst += pic->linesize[2] / 2 - width / 2;
            yend  = ydst + width;
            if (++y >= avctx->height)
                break;
        }
    }

    *got_frame = 1;
    *(AVFrame*)data= *avctx->coded_frame;

    return avpkt->size;
}

static av_cold int decode_close(AVCodecContext *avctx)
{
    AVFrame *pic = avctx->coded_frame;
    if (pic->data[0])
        avctx->release_buffer(avctx, pic);
    av_freep(&avctx->coded_frame);

    return 0;
}

AVCodec ff_v210x_decoder = {
        "v210x", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_V210X, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
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
        decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        decode_frame, /* decode */
        decode_close, /* close */
    };

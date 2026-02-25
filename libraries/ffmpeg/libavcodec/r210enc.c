/*
 * R210 encoder
 *
 * Copyright (c) 2012 Paul B Mahol
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
#include "bytestream.h"

static av_cold int encode_init(AVCodecContext *avctx)
{
    avctx->coded_frame = avcodec_alloc_frame();

    if (!avctx->coded_frame)
        return AVERROR(ENOMEM);

    return 0;
}

static int encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                        const AVFrame *pic, int *got_packet)
{
    int i, j, ret;
    int aligned_width = FFALIGN(avctx->width,
                                avctx->codec_id == AV_CODEC_ID_R10K ? 1 : 64);
    int pad = (aligned_width - avctx->width) * 4;
    uint8_t *src_line;
    uint8_t *dst;

    if ((ret = ff_alloc_packet2(avctx, pkt, 4 * aligned_width * avctx->height)) < 0)
        return ret;

    avctx->coded_frame->reference = 0;
    avctx->coded_frame->key_frame = 1;
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    src_line = pic->data[0];
    dst = pkt->data;

    for (i = 0; i < avctx->height; i++) {
        uint16_t *src = (uint16_t *)src_line;
        for (j = 0; j < avctx->width; j++) {
            uint32_t pixel;
            uint16_t r = *src++ >> 6;
            uint16_t g = *src++ >> 6;
            uint16_t b = *src++ >> 4;
            if (avctx->codec_id == AV_CODEC_ID_R210)
                pixel = (r << 20) | (g << 10) | b >> 2;
            else
                pixel = (r << 22) | (g << 12) | b;
            if (avctx->codec_id == AV_CODEC_ID_AVRP)
                bytestream_put_le32(&dst, pixel);
            else
                bytestream_put_be32(&dst, pixel);
        }
        memset(dst, 0, pad);
        dst += pad;
        src_line += pic->linesize[0];
    }

    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;
    return 0;
}

static av_cold int encode_close(AVCodecContext *avctx)
{
    av_freep(&avctx->coded_frame);

    return 0;
}

#if CONFIG_R210_ENCODER
static const enum AVPixelFormat _ff_r210enc_fmts_133[] = { AV_PIX_FMT_RGB48, AV_PIX_FMT_NONE };
AVCodec ff_r210_encoder = {
        "r210", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed RGB 10-bit"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_R210, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_r210enc_fmts_133, /* pix_fmts */
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
        encode_init, /* init */
        0, /* encode_sub */
        encode_frame, /* encode2 */
        0, /* decode */
        encode_close, /* close */
    };
#endif
#if CONFIG_R10K_ENCODER
static const enum AVPixelFormat _ff_r210enc_fmts_134[] = { AV_PIX_FMT_RGB48, AV_PIX_FMT_NONE };
AVCodec ff_r10k_encoder = {
        "r10k", /* name */
        NULL_IF_CONFIG_SMALL("AJA Kona 10-bit RGB Codec"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_R10K, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_r210enc_fmts_134, /* pix_fmts */
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
        encode_init, /* init */
        0, /* encode_sub */
        encode_frame, /* encode2 */
        0, /* decode */
        encode_close, /* close */
    };
#endif
#if CONFIG_AVRP_ENCODER
static const enum AVPixelFormat _ff_r210enc_fmts_135[] = { AV_PIX_FMT_RGB48, AV_PIX_FMT_NONE };
AVCodec ff_avrp_encoder = {
        "avrp", /* name */
        NULL_IF_CONFIG_SMALL("Avid 1:1 10-bit RGB Packer"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_AVRP, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_r210enc_fmts_135, /* pix_fmts */
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
        encode_init, /* init */
        0, /* encode_sub */
        encode_frame, /* encode2 */
        0, /* decode */
        encode_close, /* close */
    };
#endif

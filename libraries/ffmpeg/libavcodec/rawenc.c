/*
 * Raw Video Encoder
 * Copyright (c) 2001 Fabrice Bellard
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
 * Raw Video Encoder
 */

#include "avcodec.h"
#include "raw.h"
#include "internal.h"
#include "libavutil/pixdesc.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/internal.h"

static av_cold int raw_init_encoder(AVCodecContext *avctx)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(avctx->pix_fmt);

    avctx->coded_frame            = avctx->priv_data;
    avcodec_get_frame_defaults(avctx->coded_frame);
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    avctx->bits_per_coded_sample = av_get_bits_per_pixel(desc);
    if(!avctx->codec_tag)
        avctx->codec_tag = avcodec_pix_fmt_to_codec_tag(avctx->pix_fmt);
    return 0;
}

static int raw_encode(AVCodecContext *avctx, AVPacket *pkt,
                      const AVFrame *frame, int *got_packet)
{
    int ret = avpicture_get_size(avctx->pix_fmt, avctx->width, avctx->height);

    if (ret < 0)
        return ret;

    if ((ret = ff_alloc_packet2(avctx, pkt, ret)) < 0)
        return ret;
    if ((ret = avpicture_layout((const AVPicture *)frame, avctx->pix_fmt, avctx->width,
                                avctx->height, pkt->data, pkt->size)) < 0)
        return ret;

    if(avctx->codec_tag == AV_RL32("yuv2") && ret > 0 &&
       avctx->pix_fmt   == AV_PIX_FMT_YUYV422) {
        int x;
        for(x = 1; x < avctx->height*avctx->width*2; x += 2)
            pkt->data[x] ^= 0x80;
    }
    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;
    return 0;
}

AVCodec ff_rawvideo_encoder = {
        "rawvideo", /* name */
        NULL_IF_CONFIG_SMALL("raw video"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_RAWVIDEO, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(AVFrame), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        raw_init_encoder, /* init */
        0, /* encode_sub */
        raw_encode, /* encode2 */
    };

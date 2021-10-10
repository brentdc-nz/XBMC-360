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
#include "libavutil/pixdesc.h"
#include "libavutil/intreadwrite.h"

static av_cold int raw_init_encoder(AVCodecContext *avctx)
{
#ifdef _MSC_VER
	AVPixFmtDescriptor *av_pix_fmt_descriptors = get_av_pix_fmt_descriptors();
#endif

    avctx->coded_frame = (AVFrame *)avctx->priv_data;
    avctx->coded_frame->pict_type = FF_I_TYPE;
    avctx->coded_frame->key_frame = 1;
    avctx->bits_per_coded_sample = av_get_bits_per_pixel(&av_pix_fmt_descriptors[avctx->pix_fmt]);
    if(!avctx->codec_tag)
        avctx->codec_tag = avcodec_pix_fmt_to_codec_tag(avctx->pix_fmt);
    return 0;
}

static int raw_encode(AVCodecContext *avctx,
                            unsigned char *frame, int buf_size, void *data)
{
    int ret = avpicture_layout((AVPicture *)data, avctx->pix_fmt, avctx->width,
                                               avctx->height, frame, buf_size);

    if(avctx->codec_tag == AV_RL32("yuv2") && ret > 0 &&
       avctx->pix_fmt   == PIX_FMT_YUYV422) {
        int x;
        for(x = 1; x < avctx->height*avctx->width*2; x += 2)
            frame[x] ^= 0x80;
    }
    return ret;
}

AVCodec ff_rawvideo_encoder = {
#ifndef MSC_STRUCTS
    "rawvideo",
    AVMEDIA_TYPE_VIDEO,
    CODEC_ID_RAWVIDEO,
    sizeof(AVFrame),
    raw_init_encoder,
    raw_encode,
    .long_name = NULL_IF_CONFIG_SMALL("raw video"),
#else
    /* name = */ "rawvideo",
    /* type = */ AVMEDIA_TYPE_VIDEO,
    /* id = */ CODEC_ID_RAWVIDEO,
    /* priv_data_size = */ sizeof(AVFrame),
    /* init = */ raw_init_encoder,
    /* encode = */ raw_encode,
    /* close = */ 0,
    /* decode = */ 0,
    /* capabilities = */ 0,
    /* next = */ 0,
    /* flush = */ 0,
    /* supported_framerates = */ 0,
    /* pix_fmts = */ 0,
    /* long_name = */ NULL_IF_CONFIG_SMALL("raw video"),
    /* supported_samplerates = */ 0,
    /* sample_fmts = */ 0,
    /* channel_layouts = */ 0,
#endif
};

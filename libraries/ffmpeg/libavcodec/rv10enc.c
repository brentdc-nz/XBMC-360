/*
 * RV10 encoder
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer
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
 * RV10 encoder
 */

#include "mpegvideo.h"
#include "put_bits.h"

void ff_rv10_encode_picture_header(MpegEncContext *s, int picture_number)
{
    int full_frame= 0;

    avpriv_align_put_bits(&s->pb);

    put_bits(&s->pb, 1, 1);     /* marker */

    put_bits(&s->pb, 1, (s->pict_type == AV_PICTURE_TYPE_P));

    put_bits(&s->pb, 1, 0);     /* not PB frame */

    put_bits(&s->pb, 5, s->qscale);

    if (s->pict_type == AV_PICTURE_TYPE_I) {
        /* specific MPEG like DC coding not used */
    }
    /* if multiple packets per frame are sent, the position at which
       to display the macroblocks is coded here */
    if(!full_frame){
        put_bits(&s->pb, 6, 0); /* mb_x */
        put_bits(&s->pb, 6, 0); /* mb_y */
        put_bits(&s->pb, 12, s->mb_width * s->mb_height);
    }

    put_bits(&s->pb, 3, 0);     /* ignored */
}

FF_MPV_GENERIC_CLASS(rv10)

static const enum AVPixelFormat _ff_rv10enc_fmts_140[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
AVCodec ff_rv10_encoder = {
        "rv10", /* name */
        NULL_IF_CONFIG_SMALL("RealVideo 1.0"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_RV10, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        _ff_rv10enc_fmts_140, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        &rv10_class, /* priv_class */
        0, /* profiles */
        sizeof(MpegEncContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_MPV_encode_init, /* init */
        0, /* encode_sub */
        ff_MPV_encode_picture, /* encode2 */
        0, /* decode */
        ff_MPV_encode_end, /* close */
    };

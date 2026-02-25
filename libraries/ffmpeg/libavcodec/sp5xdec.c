/*
 * Sunplus JPEG decoder (SP5X)
 * Copyright (c) 2003 Alex Beregszaszi
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
 * Sunplus JPEG decoder (SP5X).
 */

#include "avcodec.h"
#include "mjpeg.h"
#include "mjpegdec.h"
#include "sp5x.h"


static int sp5x_decode_frame(AVCodecContext *avctx,
                              void *data, int *got_frame,
                              AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    AVPacket avpkt_recoded;
    const int qscale = 5;
    uint8_t *recoded;
    int i = 0, j = 0;

    if (!avctx->width || !avctx->height)
        return -1;

    recoded = av_mallocz(buf_size + 1024);
    if (!recoded)
        return -1;

    /* SOI */
    recoded[j++] = 0xFF;
    recoded[j++] = 0xD8;

    memcpy(recoded+j, &sp5x_data_dqt[0], sizeof(sp5x_data_dqt));
    memcpy(recoded+j+5, &sp5x_quant_table[qscale * 2], 64);
    memcpy(recoded+j+70, &sp5x_quant_table[(qscale * 2) + 1], 64);
    j += sizeof(sp5x_data_dqt);

    memcpy(recoded+j, &sp5x_data_dht[0], sizeof(sp5x_data_dht));
    j += sizeof(sp5x_data_dht);

    memcpy(recoded+j, &sp5x_data_sof[0], sizeof(sp5x_data_sof));
    AV_WB16(recoded+j+5, avctx->coded_height);
    AV_WB16(recoded+j+7, avctx->coded_width);
    j += sizeof(sp5x_data_sof);

    memcpy(recoded+j, &sp5x_data_sos[0], sizeof(sp5x_data_sos));
    j += sizeof(sp5x_data_sos);

    if(avctx->codec_id==AV_CODEC_ID_AMV)
        for (i = 2; i < buf_size-2 && j < buf_size+1024-2; i++)
            recoded[j++] = buf[i];
    else
    for (i = 14; i < buf_size && j < buf_size+1024-3; i++)
    {
        recoded[j++] = buf[i];
        if (buf[i] == 0xff)
            recoded[j++] = 0;
    }

    /* EOI */
    recoded[j++] = 0xFF;
    recoded[j++] = 0xD9;

    av_init_packet(&avpkt_recoded);
    avpkt_recoded.data = recoded;
    avpkt_recoded.size = j;
    i = ff_mjpeg_decode_frame(avctx, data, got_frame, &avpkt_recoded);

    av_free(recoded);

    return i < 0 ? i : avpkt->size;
}

#if CONFIG_SP5X_DECODER
AVCodec ff_sp5x_decoder = {
        "sp5x", /* name */
        NULL_IF_CONFIG_SMALL("Sunplus JPEG (SP5X)"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_SP5X, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        3, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(MJpegDecodeContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_mjpeg_decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        sp5x_decode_frame, /* decode */
        ff_mjpeg_decode_end, /* close */
    };
#endif
#if CONFIG_AMV_DECODER
AVCodec ff_amv_decoder = {
        "amv", /* name */
        NULL_IF_CONFIG_SMALL("AMV Video"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_AMV, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(MJpegDecodeContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_mjpeg_decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        sp5x_decode_frame, /* decode */
        ff_mjpeg_decode_end, /* close */
    };
#endif

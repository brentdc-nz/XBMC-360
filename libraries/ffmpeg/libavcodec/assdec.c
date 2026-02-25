/*
 * SSA/ASS decoder
 * Copyright (c) 2010  Aurelien Jacobs <aurel@gnuage.org>
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

#include <string.h>

#include "avcodec.h"
#include "ass.h"
#include "ass_split.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

static av_cold int ass_decode_init(AVCodecContext *avctx)
{
    avctx->subtitle_header = av_malloc(avctx->extradata_size + 1);
    if (!avctx->subtitle_header)
        return AVERROR(ENOMEM);
    memcpy(avctx->subtitle_header, avctx->extradata, avctx->extradata_size);
    avctx->subtitle_header[avctx->extradata_size] = 0;
    avctx->subtitle_header_size = avctx->extradata_size;
    avctx->priv_data = ff_ass_split(avctx->extradata);
    if(!avctx->priv_data)
        return -1;
    return 0;
}

static int ass_decode_frame(AVCodecContext *avctx, void *data, int *got_sub_ptr,
                            AVPacket *avpkt)
{
    const char *ptr = avpkt->data;
    int len, size = avpkt->size;

    while (size > 0) {
        int duration;
        ASSDialog *dialog = ff_ass_split_dialog(avctx->priv_data, ptr, 0, NULL);
        if (!dialog)
            return AVERROR_INVALIDDATA;
        duration = dialog->end - dialog->start;
        len = ff_ass_add_rect(data, ptr, 0, duration, 1);
        if (len < 0)
            return len;
        ptr  += len;
        size -= len;
    }

    *got_sub_ptr = avpkt->size > 0;
    return avpkt->size;
}

static int ass_decode_close(AVCodecContext *avctx)
{
    ff_ass_split_free(avctx->priv_data);
    avctx->priv_data = NULL;
    return 0;
}

AVCodec ff_ass_decoder = {
        "ass", /* name */
        NULL_IF_CONFIG_SMALL("SSA (SubStation Alpha) subtitle"), /* long_name */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        AV_CODEC_ID_SSA, /* id */
        0, /* capabilities */
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
        ass_decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        ass_decode_frame, /* decode */
        ass_decode_close, /* close */
    };

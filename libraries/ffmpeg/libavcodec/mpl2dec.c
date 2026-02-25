/*
 * Copyright (c) 2012 Clément Bœsch
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
 * MPL2 subtitles decoder
 *
 * @see http://web.archive.org/web/20090328040233/http://napisy.ussbrowarek.org/mpl2-eng.html
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"

static int mpl2_event_to_ass(AVBPrint *buf, const char *p)
{
    if (*p == ' ')
        p++;

    while (*p) {
        int got_style = 0;

        while (*p && strchr("/\\_", *p)) {
            if      (*p == '/')  av_bprintf(buf, "{\\i1}");
            else if (*p == '\\') av_bprintf(buf, "{\\b1}");
            else if (*p == '_')  av_bprintf(buf, "{\\u1}");
            got_style = 1;
            p++;
        }

        while (*p && *p != '|') {
            if (*p != '\r' && *p != '\n')
                av_bprint_chars(buf, *p, 1);
            p++;
        }

        if (*p == '|') {
            if (got_style)
                av_bprintf(buf, "{\\r}");
            av_bprintf(buf, "\\N");
            p++;
        }
    }

    av_bprintf(buf, "\r\n");
    return 0;
}

static int mpl2_decode_frame(AVCodecContext *avctx, void *data,
                             int *got_sub_ptr, AVPacket *avpkt)
{
    AVBPrint buf;
    AVSubtitle *sub = data;
    const char *ptr = avpkt->data;
    const int ts_start     = av_rescale_q(avpkt->pts,      avctx->time_base, av_make_q(1, 100));
    const int ts_duration  = avpkt->duration != -1 ?
                             av_rescale_q(avpkt->duration, avctx->time_base, av_make_q(1, 100)) : -1;

    av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);
    if (ptr && avpkt->size > 0 && *ptr && !mpl2_event_to_ass(&buf, ptr)) {
        if (!av_bprint_is_complete(&buf)) {
            av_bprint_finalize(&buf, NULL);
            return AVERROR(ENOMEM);
        }
        ff_ass_add_rect(sub, buf.str, ts_start, ts_duration, 0);
    }
    *got_sub_ptr = sub->num_rects > 0;
    av_bprint_finalize(&buf, NULL);
    return avpkt->size;
}

AVCodec ff_mpl2_decoder = {
        "mpl2", /* name */
        NULL_IF_CONFIG_SMALL("MPL2 subtitle"), /* long_name */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        AV_CODEC_ID_MPL2, /* id */
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
        ff_ass_subtitle_header_default, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        mpl2_decode_frame, /* decode */
    };

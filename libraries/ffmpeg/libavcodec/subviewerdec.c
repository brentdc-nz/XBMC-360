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
 * SubViewer subtitle decoder
 * @see https://en.wikipedia.org/wiki/SubViewer
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"

static int subviewer_event_to_ass(AVBPrint *buf, const char *p)
{
    while (*p) {
        if (!strncmp(p, "[br]", 4)) {
            av_bprintf(buf, "\\N");
            p += 4;
        } else {
            if (p[0] == '\n' && p[1])
                av_bprintf(buf, "\\N");
            else if (*p != '\n' && *p != '\r')
                av_bprint_chars(buf, *p, 1);
            p++;
        }
    }

    av_bprintf(buf, "\r\n");
    return 0;
}

static int subviewer_decode_frame(AVCodecContext *avctx,
                                  void *data, int *got_sub_ptr, AVPacket *avpkt)
{
    char c;
    AVSubtitle *sub = data;
    const char *ptr = avpkt->data;
    AVBPrint buf;

    /* To be removed later */
    if (ptr && sscanf(ptr, "%*u:%*u:%*u.%*u,%*u:%*u:%*u.%*u%c", &c) == 1) {
        av_log(avctx, AV_LOG_ERROR, "AVPacket is not clean (contains timing "
               "information). You need to upgrade your libavformat or "
               "sanitize your packet.\n");
        return AVERROR_INVALIDDATA;
    }

    av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);
    // note: no need to rescale pts & duration since they are in the same
    // timebase as ASS (1/100)
    if (ptr && avpkt->size > 0 && !subviewer_event_to_ass(&buf, ptr))
        ff_ass_add_rect(sub, buf.str, avpkt->pts, avpkt->duration, 0);
    *got_sub_ptr = sub->num_rects > 0;
    av_bprint_finalize(&buf, NULL);
    return avpkt->size;
}

AVCodec ff_subviewer_decoder = {
        "subviewer", /* name */
        NULL_IF_CONFIG_SMALL("SubViewer subtitle"), /* long_name */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        AV_CODEC_ID_SUBVIEWER, /* id */
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
        subviewer_decode_frame, /* decode */
    };

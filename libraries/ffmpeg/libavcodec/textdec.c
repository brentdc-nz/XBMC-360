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
 * Raw subtitles decoder
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"

typedef struct {
    AVClass *class;
    const char *linebreaks;
    int keep_ass_markup;
} TextContext;

#define OFFSET(x) offsetof(TextContext, x)
#define SD AV_OPT_FLAG_SUBTITLE_PARAM | AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        SD, /* flags */
    },
    { NULL }
};

static int text_event_to_ass(const AVCodecContext *avctx, AVBPrint *buf,
                             const char *p, const char *p_end)
{
    const TextContext *text = avctx->priv_data;

    for (; p < p_end && *p; p++) {

        /* forced custom line breaks, not accounted as "normal" EOL */
        if (text->linebreaks && strchr(text->linebreaks, *p)) {
            av_bprintf(buf, "\\N");

        /* standard ASS escaping so random characters don't get mis-interpreted
         * as ASS */
        } else if (!text->keep_ass_markup && strchr("{}\\", *p)) {
            av_bprintf(buf, "\\%c", *p);

        /* some packets might end abruptly (no \0 at the end, like for example
         * in some cases of demuxing from a classic video container), some
         * might be terminated with \n or \r\n which we have to remove (for
         * consistency with those who haven't), and we also have to deal with
         * evil cases such as \r at the end of the buffer (and no \0 terminated
         * character) */
        } else if (p[0] == '\n') {
            /* some stuff left so we can insert a line break */
            if (p < p_end - 1)
                av_bprintf(buf, "\\N");
        } else if (p[0] == '\r' && p < p_end - 1 && p[1] == '\n') {
            /* \r followed by a \n, we can skip it. We don't insert the \N yet
             * because we don't know if it is followed by more text */
            continue;

        /* finally, a sane character */
        } else {
            av_bprint_chars(buf, *p, 1);
        }
    }
    av_bprintf(buf, "\r\n");
    return 0;
}

static int text_decode_frame(AVCodecContext *avctx, void *data,
                             int *got_sub_ptr, AVPacket *avpkt)
{
    AVBPrint buf;
    AVSubtitle *sub = data;
    const char *ptr = avpkt->data;
    const int ts_start     = av_rescale_q(avpkt->pts,      avctx->time_base, av_make_q(1, 100));
    const int ts_duration  = avpkt->duration != -1 ?
                             av_rescale_q(avpkt->duration, avctx->time_base, av_make_q(1, 100)) : -1;

    av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);
    if (ptr && avpkt->size > 0 && *ptr &&
        !text_event_to_ass(avctx, &buf, ptr, ptr + avpkt->size)) {
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

#define DECLARE_CLASS(decname) static const AVClass decname ## _decoder_class = {   \
    /* class_name */ #decname " decoder",      \
    /* item_name */ av_default_item_name,     \
    /* option */ decname ## _options,      \
    /* version */ LIBAVUTIL_VERSION_INT,    \
}

#if CONFIG_TEXT_DECODER
#define text_options options
DECLARE_CLASS(text);

AVCodec ff_text_decoder = {
        "text", /* name */
        NULL_IF_CONFIG_SMALL("Raw text subtitle"), /* long_name */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        AV_CODEC_ID_TEXT, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        &text_decoder_class, /* priv_class */
        0, /* profiles */
        sizeof(TextContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ff_ass_subtitle_header_default, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        text_decode_frame, /* decode */
    };
#endif

#if CONFIG_VPLAYER_DECODER || CONFIG_PJS_DECODER || CONFIG_SUBVIEWER1_DECODER

static int linebreak_init(AVCodecContext *avctx)
{
    TextContext *text = avctx->priv_data;
    text->linebreaks = "|";
    return ff_ass_subtitle_header_default(avctx);
}

#if CONFIG_VPLAYER_DECODER
#define vplayer_options options
DECLARE_CLASS(vplayer);

AVCodec ff_vplayer_decoder = {
        "vplayer", /* name */
        NULL_IF_CONFIG_SMALL("VPlayer subtitle"), /* long_name */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        AV_CODEC_ID_VPLAYER, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        &vplayer_decoder_class, /* priv_class */
        0, /* profiles */
        sizeof(TextContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        linebreak_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        text_decode_frame, /* decode */
    };
#endif

#if CONFIG_PJS_DECODER
#define pjs_options options
DECLARE_CLASS(pjs);

AVCodec ff_pjs_decoder = {
        "pjs", /* name */
        NULL_IF_CONFIG_SMALL("PJS subtitle"), /* long_name */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        AV_CODEC_ID_PJS, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        &pjs_decoder_class, /* priv_class */
        0, /* profiles */
        sizeof(TextContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        linebreak_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        text_decode_frame, /* decode */
    };
#endif

#if CONFIG_SUBVIEWER1_DECODER
#define subviewer1_options options
DECLARE_CLASS(subviewer1);

AVCodec ff_subviewer1_decoder = {
        "subviewer1", /* name */
        NULL_IF_CONFIG_SMALL("SubViewer1 subtitle"), /* long_name */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        AV_CODEC_ID_SUBVIEWER1, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        &subviewer1_decoder_class, /* priv_class */
        0, /* profiles */
        sizeof(TextContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        linebreak_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        text_decode_frame, /* decode */
    };
#endif

#endif /* text subtitles with '|' line break */

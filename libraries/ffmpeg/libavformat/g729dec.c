/*
 * G.729 raw format demuxer
 * Copyright (c) 2011 Vladimir Voroshilov
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

#include "avformat.h"
#include "internal.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"

typedef struct G729DemuxerContext {
    AVClass *class;
    int bit_rate;
} G729DemuxerContext;

static int g729_read_header(AVFormatContext *s)
{
    AVStream* st;
    G729DemuxerContext *s1 = s->priv_data;

    st = avformat_new_stream(s, NULL);
    if (!st)
        return AVERROR(ENOMEM);

    st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id = AV_CODEC_ID_G729;
    st->codec->sample_rate = 8000;
    st->codec->channels = 1;

    if (s1 && s1->bit_rate) {
        s->bit_rate = s1->bit_rate;
    }

    if (s->bit_rate == 0) {
        av_log(s, AV_LOG_DEBUG, "No bitrate specified. Assuming 8000 b/s\n");
        s->bit_rate = 8000;
    }

    if (s->bit_rate == 6400) {
        st->codec->block_align = 8;
    } else if (s->bit_rate == 8000) {
        st->codec->block_align = 10;
    } else {
        av_log(s, AV_LOG_ERROR, "Only 8000 b/s and 6400 b/s bitrates are supported. Provided: %d b/s\n", s->bit_rate);
        return AVERROR_INVALIDDATA;
    }

    avpriv_set_pts_info(st, st->codec->block_align << 3, 1, st->codec->sample_rate);
    return 0;
}
static int g729_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret;

    ret = av_get_packet(s->pb, pkt, s->streams[0]->codec->block_align);

    pkt->stream_index = 0;
    if (ret < 0)
        return ret;

    pkt->dts = pkt->pts = pkt->pos / s->streams[0]->codec->block_align;

    return ret;
}

static const AVOption g729_options[] = {
    { "bit_rate", "", offsetof(G729DemuxerContext, bit_rate), AV_OPT_TYPE_INT, { 0 }, 0, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass g729_demuxer_class = {
    "g729 demuxer", /* class_name */
    av_default_item_name, /* item_name */
    g729_options, /* option */
    LIBAVUTIL_VERSION_INT, /* version */
};

AVInputFormat ff_g729_demuxer = {
    "g729", /* name */
    NULL_IF_CONFIG_SMALL("G.729 raw format demuxer"), /* long_name */
    AVFMT_GENERIC_INDEX, /* flags */
    "g729", /* extensions */
    0, /* codec_tag */
    &g729_demuxer_class, /* priv_class */
    0, /* next */
    0, /* raw_codec_id */
    sizeof(G729DemuxerContext), /* priv_data_size */
    0, /* read_probe */
    g729_read_header, /* read_header */
    g729_read_packet, /* read_packet */
};

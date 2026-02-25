/*
 * IRCAM muxer
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

#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "avio_internal.h"
#include "internal.h"
#include "rawenc.h"
#include "ircam.h"

static int ircam_write_header(AVFormatContext *s)
{
    AVCodecContext *codec = s->streams[0]->codec;
    uint32_t tag;

    if (s->nb_streams != 1) {
        av_log(s, AV_LOG_ERROR, "only one stream is supported\n");
        return AVERROR(EINVAL);
    }

    tag = ff_codec_get_tag(ff_codec_ircam_le_tags, codec->codec_id);
    if (!tag) {
        av_log(s, AV_LOG_ERROR, "unsupported codec\n");
        return AVERROR(EINVAL);
    }

    avio_wl32(s->pb, 0x0001A364);
    avio_wl32(s->pb, av_float2int(codec->sample_rate));
    avio_wl32(s->pb, codec->channels);
    avio_wl32(s->pb, tag);
    ffio_fill(s->pb, 0, 1008);
    return 0;
}

static const AVCodecTag *const _ff_ircamenc_tags_13[] = { ff_codec_ircam_le_tags, 0 };
AVOutputFormat ff_ircam_muxer = {
    "ircam", /* name */
    NULL_IF_CONFIG_SMALL("Berkeley/IRCAM/CARL Sound Format"), /* long_name */
    0, /* mime_type */
    "sf,ircam", /* extensions */
    AV_CODEC_ID_PCM_S16LE, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    0, /* flags */
    _ff_ircamenc_tags_13, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    ircam_write_header, /* write_header */
    ff_raw_write_packet, /* write_packet */
};

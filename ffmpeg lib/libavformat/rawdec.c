/*
 * RAW demuxers
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2005 Alex Beregszaszi
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
#include <xtl.h>
#include <assert.h>
#include "avformat.h"
#include "avio_internal.h"
#include "rawdec.h"

/* raw input */
int ff_raw_read_header(AVFormatContext *s, AVFormatParameters *ap)
{
    AVStream *st;
    enum CodecID id;

    st = av_new_stream(s, 0);
    if (!st)
        return AVERROR(ENOMEM);

        id = s->iformat->value;
        if (id == CODEC_ID_RAWVIDEO) {
            st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
        } else {
            st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
        }
        st->codec->codec_id = id;

        switch(st->codec->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            st->codec->sample_rate = ap->sample_rate;
            if(ap->channels) st->codec->channels = ap->channels;
            else             st->codec->channels = 1;
            st->codec->bits_per_coded_sample = av_get_bits_per_sample(st->codec->codec_id);
            assert(st->codec->bits_per_coded_sample > 0);
            st->codec->block_align = st->codec->bits_per_coded_sample*st->codec->channels/8;
            av_set_pts_info(st, 64, 1, st->codec->sample_rate);
            break;
        case AVMEDIA_TYPE_VIDEO:
            if(ap->time_base.num)
                av_set_pts_info(st, 64, ap->time_base.num, ap->time_base.den);
            else
                av_set_pts_info(st, 64, 1, 25);
            st->codec->width = ap->width;
            st->codec->height = ap->height;
            st->codec->pix_fmt = ap->pix_fmt;
            if(st->codec->pix_fmt == PIX_FMT_NONE)
                st->codec->pix_fmt= PIX_FMT_YUV420P;
            break;
        default:
            return -1;
        }
    return 0;
}

#define RAW_PACKET_SIZE 1024

int ff_raw_read_partial_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret, size;

    size = RAW_PACKET_SIZE;

    if (av_new_packet(pkt, size) < 0)
        return AVERROR(ENOMEM);

    pkt->pos= avio_tell(s->pb);
    pkt->stream_index = 0;
    ret = ffio_read_partial(s->pb, pkt->data, size);
    if (ret < 0) {
        av_free_packet(pkt);
        return ret;
    }
    pkt->size = ret;
    return ret;
}

int ff_raw_audio_read_header(AVFormatContext *s,
                             AVFormatParameters *ap)
{
    AVStream *st = av_new_stream(s, 0);
    if (!st)
        return AVERROR(ENOMEM);
    st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id = s->iformat->value;
    st->need_parsing = AVSTREAM_PARSE_FULL;
    /* the parameters will be extracted from the compressed bitstream */

    return 0;
}

/* MPEG-1/H.263 input */
int ff_raw_video_read_header(AVFormatContext *s,
                             AVFormatParameters *ap)
{
    AVStream *st;

    st = av_new_stream(s, 0);
    if (!st)
        return AVERROR(ENOMEM);

    st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    st->codec->codec_id = s->iformat->value;
    st->need_parsing = AVSTREAM_PARSE_FULL;

    /* for MJPEG, specify frame rate */
    /* for MPEG-4 specify it, too (most MPEG-4 streams do not have the fixed_vop_rate set ...)*/
    if (ap->time_base.num) {
        st->codec->time_base= ap->time_base;
    } else if ( st->codec->codec_id == CODEC_ID_MJPEG ||
                st->codec->codec_id == CODEC_ID_MPEG4 ||
                st->codec->codec_id == CODEC_ID_DIRAC ||
                st->codec->codec_id == CODEC_ID_DNXHD ||
                st->codec->codec_id == CODEC_ID_VC1   ||
                st->codec->codec_id == CODEC_ID_H264) {
					st->codec->time_base= av_create_rational(1,25);
    }
    av_set_pts_info(st, 64, 1, 1200000);

    return 0;
}

/* Note: Do not forget to add new entries to the Makefile as well. */

#if CONFIG_G722_DEMUXER
AVInputFormat ff_g722_demuxer = {
    "g722",
    NULL_IF_CONFIG_SMALL("raw G.722"),
    0,
    NULL,
    ff_raw_read_header,
    ff_raw_read_partial_packet,
    .flags= AVFMT_GENERIC_INDEX,
    .extensions = "g722,722",
    .value = CODEC_ID_ADPCM_G722,
};
#endif

#if CONFIG_GSM_DEMUXER
AVInputFormat ff_gsm_demuxer = {
#ifndef MSC_STRUCTS
    "gsm",
    NULL_IF_CONFIG_SMALL("raw GSM"),
    0,
    NULL,
    audio_read_header,
    ff_raw_read_partial_packet,
    .flags= AVFMT_GENERIC_INDEX,
    .extensions = "gsm",
    .value = CODEC_ID_GSM,
};
#else
	"gsm",
	NULL_IF_CONFIG_SMALL("raw GSM"),
	0,
	NULL,
	ff_raw_audio_read_header,
	ff_raw_read_partial_packet,
	/*read_close = */ 0,
	/*read_seek = */ 0,
	/*read_timestamp = */ 0,
	/*flags = */ AVFMT_GENERIC_INDEX,
	/*extensions = */ "gsm",
	/*value = */ CODEC_ID_GSM,
	/*read_play = */ 0,
	/*read_pause = */ 0,
	/*codec_tag = */ 0,
	/*read_seek2 = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

#if CONFIG_MJPEG_DEMUXER
AVInputFormat ff_mjpeg_demuxer = {
#ifndef MSC_STRUCTS
    "mjpeg",
    NULL_IF_CONFIG_SMALL("raw MJPEG video"),
    0,
    NULL,
    video_read_header,
    ff_raw_read_partial_packet,
    .flags= AVFMT_GENERIC_INDEX,
    .extensions = "mjpg,mjpeg",
    .value = CODEC_ID_MJPEG,
};
#else
	"mjpeg",
	NULL_IF_CONFIG_SMALL("raw MJPEG video"),
	0,
	NULL,
	ff_raw_video_read_header,
	ff_raw_read_partial_packet,
	/*read_close = */ 0,
	/*read_seek = */ 0,
	/*read_timestamp = */ 0,
	/*flags = */ AVFMT_GENERIC_INDEX,
	/*extensions = */ "mjpg,mjpeg",
	/*value = */ CODEC_ID_MJPEG,
	/*read_play = */ 0,
	/*read_pause = */ 0,
	/*codec_tag = */ 0,
	/*read_seek2 = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

#if CONFIG_MLP_DEMUXER
AVInputFormat ff_mlp_demuxer = {
#ifndef MSC_STRUCTS
    "mlp",
    NULL_IF_CONFIG_SMALL("raw MLP"),
    0,
    NULL,
    audio_read_header,
    ff_raw_read_partial_packet,
    .flags= AVFMT_GENERIC_INDEX,
    .extensions = "mlp",
    .value = CODEC_ID_MLP,
};
#else
	"mlp",
	NULL_IF_CONFIG_SMALL("raw MLP"),
	0,
	NULL,
	ff_raw_audio_read_header,
	ff_raw_read_partial_packet,
	/*read_close = */ 0,
	/*read_seek = */ 0,
	/*read_timestamp = */ 0,
	/*flags = */ AVFMT_GENERIC_INDEX,
	/*extensions = */ "mlp",
	/*value = */ CODEC_ID_MLP,
	/*read_play = */ 0,
	/*read_pause = */ 0,
	/*codec_tag = */ 0,
	/*read_seek2 = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

#if CONFIG_TRUEHD_DEMUXER
AVInputFormat ff_truehd_demuxer = {
#ifndef MSC_STRUCTS
    "truehd",
    NULL_IF_CONFIG_SMALL("raw TrueHD"),
    0,
    NULL,
    audio_read_header,
    ff_raw_read_partial_packet,
    .flags= AVFMT_GENERIC_INDEX,
    .extensions = "thd",
    .value = CODEC_ID_TRUEHD,
};
#else
	"truehd",
	NULL_IF_CONFIG_SMALL("raw TrueHD"),
	0,
	NULL,
	ff_raw_audio_read_header,
	ff_raw_read_partial_packet,
	/*read_close = */ 0,
	/*read_seek = */ 0,
	/*read_timestamp = */ 0,
	/*flags = */ AVFMT_GENERIC_INDEX,
	/*extensions = */  "thd",
	/*value = */ CODEC_ID_TRUEHD,
	/*read_play = */ 0,
	/*read_pause = */ 0,
	/*codec_tag = */ 0,
	/*read_seek2 = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

#if CONFIG_SHORTEN_DEMUXER
AVInputFormat ff_shorten_demuxer = {
#ifndef MSC_STRUCTS
    "shn",
    NULL_IF_CONFIG_SMALL("raw Shorten"),
    0,
    NULL,
    audio_read_header,
    ff_raw_read_partial_packet,
    .flags= AVFMT_GENERIC_INDEX,
    .extensions = "shn",
    .value = CODEC_ID_SHORTEN,
};
#else
	"shn",
	NULL_IF_CONFIG_SMALL("raw Shorten"),
	0,
	NULL,
	ff_raw_audio_read_header,
	ff_raw_read_partial_packet,
	/*read_close = */ 0,
	/*read_seek = */ 0,
	/*read_timestamp = */ 0,
	/*flags = */ AVFMT_GENERIC_INDEX,
	/*extensions = */ "shn",
	/*value = */ CODEC_ID_SHORTEN,
	/*read_play = */ 0,
	/*read_pause = */ 0,
	/*codec_tag = */ 0,
	/*read_seek2 = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

#if CONFIG_VC1_DEMUXER
AVInputFormat ff_vc1_demuxer = {
#ifndef MSC_STRUCTS
    "vc1",
    NULL_IF_CONFIG_SMALL("raw VC-1"),
    0,
    NULL /* vc1_probe */,
    ff_raw_video_read_header,
    ff_raw_read_partial_packet,
    .extensions = "vc1",
    .value = CODEC_ID_VC1,
};
#else
	"vc1",
	NULL_IF_CONFIG_SMALL("raw VC-1"),
	0,
	NULL /* vc1_probe */,
	ff_raw_video_read_header,
	ff_raw_read_partial_packet,
	/*read_close = */ 0,
	/*read_seek = */ 0,
	/*read_timestamp = */ 0,
	/*flags = */ 0,
	/*extensions = */ "vc1",
	/*value = */ CODEC_ID_VC1,
	/*read_play = */ 0,
	/*read_pause = */ 0,
	/*codec_tag = */ 0,
	/*read_seek2 = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

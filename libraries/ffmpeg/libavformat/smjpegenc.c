/*
 * SMJPEG muxer
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

/**
 * @file
 * This is a muxer for Loki SDL Motion JPEG files
 */

#include "avformat.h"
#include "internal.h"
#include "smjpeg.h"

typedef struct SMJPEGMuxContext {
    uint32_t duration;
} SMJPEGMuxContext;

static int smjpeg_write_header(AVFormatContext *s)
{
    AVDictionaryEntry *t = NULL;
    AVIOContext *pb = s->pb;
    int n, tag;

    if (s->nb_streams > 2) {
        av_log(s, AV_LOG_ERROR, "more than >2 streams are not supported\n");
        return AVERROR(EINVAL);
    }
    avio_write(pb, SMJPEG_MAGIC, 8);
    avio_wb32(pb, 0);
    avio_wb32(pb, 0);

    while ((t = av_dict_get(s->metadata, "", t, AV_DICT_IGNORE_SUFFIX))) {
        avio_wl32(pb, SMJPEG_TXT);
        avio_wb32(pb, strlen(t->key) + strlen(t->value) + 3);
        avio_write(pb, t->key, strlen(t->key));
        avio_write(pb, " = ", 3);
        avio_write(pb, t->value, strlen(t->value));
    }

    for (n = 0; n < s->nb_streams; n++) {
        AVStream *st = s->streams[n];
        AVCodecContext *codec = st->codec;
        if (codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            tag = ff_codec_get_tag(ff_codec_smjpeg_audio_tags, codec->codec_id);
            if (!tag) {
                av_log(s, AV_LOG_ERROR, "unsupported audio codec\n");
                return AVERROR(EINVAL);
            }
            avio_wl32(pb, SMJPEG_SND);
            avio_wb32(pb, 8);
            avio_wb16(pb, codec->sample_rate);
            avio_w8(pb, codec->bits_per_coded_sample);
            avio_w8(pb, codec->channels);
            avio_wl32(pb, tag);
            avpriv_set_pts_info(st, 32, 1, 1000);
        } else if (codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            tag = ff_codec_get_tag(ff_codec_smjpeg_video_tags, codec->codec_id);
            if (!tag) {
                av_log(s, AV_LOG_ERROR, "unsupported video codec\n");
                return AVERROR(EINVAL);
            }
            avio_wl32(pb, SMJPEG_VID);
            avio_wb32(pb, 12);
            avio_wb32(pb, 0);
            avio_wb16(pb, codec->width);
            avio_wb16(pb, codec->height);
            avio_wl32(pb, tag);
            avpriv_set_pts_info(st, 32, 1, 1000);
        }
    }

    avio_wl32(pb, SMJPEG_HEND);
    avio_flush(pb);

    return 0;
}

static int smjpeg_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    SMJPEGMuxContext *smc = s->priv_data;
    AVIOContext *pb = s->pb;
    AVStream *st = s->streams[pkt->stream_index];
    AVCodecContext *codec = st->codec;

    if (codec->codec_type == AVMEDIA_TYPE_AUDIO)
        avio_wl32(pb, SMJPEG_SNDD);
    else if (codec->codec_type == AVMEDIA_TYPE_VIDEO)
        avio_wl32(pb, SMJPEG_VIDD);
    else
        return 0;

    avio_wb32(pb, pkt->pts);
    avio_wb32(pb, pkt->size);
    avio_write(pb, pkt->data, pkt->size);
    avio_flush(pb);

    smc->duration = FFMAX(smc->duration, pkt->pts + pkt->duration);
    return 0;
}

static int smjpeg_write_trailer(AVFormatContext *s)
{
    SMJPEGMuxContext *smc = s->priv_data;
    AVIOContext *pb = s->pb;
    int64_t currentpos;

    if (pb->seekable) {
        currentpos = avio_tell(pb);
        avio_seek(pb, 12, SEEK_SET);
        avio_wb32(pb, smc->duration);
        avio_seek(pb, currentpos, SEEK_SET);
    }

    avio_wl32(pb, SMJPEG_DONE);

    return 0;
}

static const AVCodecTag *const _ff_smjpegenc_tags_33[] = { ff_codec_smjpeg_video_tags, ff_codec_smjpeg_audio_tags, 0 };
AVOutputFormat ff_smjpeg_muxer = {
    "smjpeg", /* name */
    NULL_IF_CONFIG_SMALL("Loki SDL MJPEG"), /* long_name */
    0, /* mime_type */
    0, /* extensions */
    AV_CODEC_ID_PCM_S16LE, /* audio_codec */
    AV_CODEC_ID_MJPEG, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_GLOBALHEADER | AVFMT_TS_NONSTRICT, /* flags */
    _ff_smjpegenc_tags_33, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    sizeof(SMJPEGMuxContext), /* priv_data_size */
    smjpeg_write_header, /* write_header */
    smjpeg_write_packet, /* write_packet */
    smjpeg_write_trailer, /* write_trailer */
};

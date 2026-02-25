/*
 * RSO muxer
 * Copyright (c) 2001 Fabrice Bellard (original AU code)
 * Copyright (c) 2010 Rafael Carre
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
#include "riff.h"
#include "rso.h"

static int rso_write_header(AVFormatContext *s)
{
    AVIOContext  *pb  = s->pb;
    AVCodecContext *enc = s->streams[0]->codec;

    if (!enc->codec_tag)
        return AVERROR_INVALIDDATA;

    if (enc->channels != 1) {
        av_log(s, AV_LOG_ERROR, "RSO only supports mono\n");
        return AVERROR_INVALIDDATA;
    }

    if (!s->pb->seekable) {
        av_log(s, AV_LOG_ERROR, "muxer does not support non seekable output\n");
        return AVERROR_INVALIDDATA;
    }

    /* XXX: find legal sample rates (if any) */
    if (enc->sample_rate >= 1u<<16) {
        av_log(s, AV_LOG_ERROR, "Sample rate must be < 65536\n");
        return AVERROR_INVALIDDATA;
    }

    if (enc->codec_id == AV_CODEC_ID_ADPCM_IMA_WAV) {
        av_log(s, AV_LOG_ERROR, "ADPCM in RSO not implemented\n");
        return AVERROR_PATCHWELCOME;
    }

    /* format header */
    avio_wb16(pb, enc->codec_tag);   /* codec ID */
    avio_wb16(pb, 0);                /* data size, will be written at EOF */
    avio_wb16(pb, enc->sample_rate);
    avio_wb16(pb, 0x0000);           /* play mode ? (0x0000 = don't loop) */

    avio_flush(pb);

    return 0;
}

static int rso_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    avio_write(s->pb, pkt->data, pkt->size);
    return 0;
}

static int rso_write_trailer(AVFormatContext *s)
{
    AVIOContext *pb = s->pb;
    int64_t file_size;
    uint16_t coded_file_size;

    file_size = avio_tell(pb);

    if (file_size < 0)
        return file_size;

    if (file_size > 0xffff + RSO_HEADER_SIZE) {
        av_log(s, AV_LOG_WARNING,
               "Output file is too big (%"PRId64" bytes >= 64kB)\n", file_size);
        coded_file_size = 0xffff;
    } else {
        coded_file_size = file_size - RSO_HEADER_SIZE;
    }

    /* update file size */
    avio_seek(pb, 2, SEEK_SET);
    avio_wb16(pb, coded_file_size);
    avio_seek(pb, file_size, SEEK_SET);

    return 0;
}

static const AVCodecTag *const _ff_rsoenc_tags_32[] = { ff_codec_rso_tags, 0 };
AVOutputFormat ff_rso_muxer = {
    "rso", /* name */
    NULL_IF_CONFIG_SMALL("Lego Mindstorms RSO"), /* long_name */
    0, /* mime_type */
    "rso", /* extensions */
    AV_CODEC_ID_PCM_U8, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    0, /* flags */
    _ff_rsoenc_tags_32, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    rso_write_header, /* write_header */
    rso_write_packet, /* write_packet */
    rso_write_trailer, /* write_trailer */
};

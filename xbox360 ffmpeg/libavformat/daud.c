/*
 * D-Cinema audio demuxer
 * Copyright (c) 2005 Reimar Döffinger
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

static int daud_header(AVFormatContext *s, AVFormatParameters *ap) {
    AVStream *st = av_new_stream(s, 0);
    if (!st)
        return AVERROR(ENOMEM);
    st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id = CODEC_ID_PCM_S24DAUD;
    st->codec->codec_tag = MKTAG('d', 'a', 'u', 'd');
    st->codec->channels = 6;
    st->codec->sample_rate = 96000;
    st->codec->bit_rate = 3 * 6 * 96000 * 8;
    st->codec->block_align = 3 * 6;
    st->codec->bits_per_coded_sample = 24;
    return 0;
}

static int daud_packet(AVFormatContext *s, AVPacket *pkt) {
    AVIOContext *pb = s->pb;
    int ret, size;
    if (url_feof(pb))
        return AVERROR(EIO);
    size = avio_rb16(pb);
    avio_rb16(pb); // unknown
    ret = av_get_packet(pb, pkt, size);
    pkt->stream_index = 0;
    return ret;
}

static int daud_write_header(struct AVFormatContext *s)
{
    AVCodecContext *codec = s->streams[0]->codec;
    if (codec->channels!=6 || codec->sample_rate!=96000)
        return -1;
    return 0;
}

static int daud_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
    if (pkt->size > 65535) {
        av_log(s, AV_LOG_ERROR,
               "Packet size too large for s302m. (%d > 65535)\n", pkt->size);
        return -1;
    }
    avio_wb16(s->pb, pkt->size);
    avio_wb16(s->pb, 0x8010); // unknown
    avio_write(s->pb, pkt->data, pkt->size);
    avio_flush(s->pb);
    return 0;
}

#if CONFIG_DAUD_DEMUXER
AVInputFormat daud_demuxer = {
#ifndef MSC_STRUCTS
    "daud",
    NULL_IF_CONFIG_SMALL("D-Cinema audio format"),
    0,
    NULL,
    daud_header,
    daud_packet,
    NULL,
    NULL,
    .extensions = "302",
};
#else
	"daud",
	NULL_IF_CONFIG_SMALL("D-Cinema audio format"),
	0,
	NULL,
	daud_header,
	daud_packet,
	/*read_close = */ 0,
	/*read_seek = */ 0,
	/*read_timestamp = */ 0,
	/*flags = */ 0,
	/*extensions = */ "302",
	/*value = */ 0,
	/*read_play = */ 0,
	/*read_pause = */ 0,
	/*codec_tag = */ 0,
	/*read_seek2 = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

#if CONFIG_DAUD_MUXER
AVOutputFormat daud_muxer = {
#ifndef MSC_STRUCTS
    "daud",
    NULL_IF_CONFIG_SMALL("D-Cinema audio format"),
    NULL,
    "302",
    0,
    CODEC_ID_PCM_S24DAUD,
    CODEC_ID_NONE,
    daud_write_header,
    daud_write_packet,
    .flags= AVFMT_NOTIMESTAMPS,
};
#else
	"daud",
	NULL_IF_CONFIG_SMALL("D-Cinema audio format"),
	NULL,
	"302",
	0,
	CODEC_ID_PCM_S24DAUD,
	CODEC_ID_NONE,
	daud_write_header,
	daud_write_packet,
	/*write_trailer = */ 0,
	/*flags = */ AVFMT_NOTIMESTAMPS,
	/*set_parameters = */ 0,
	/*interleave_packet = */ 0,
	/*codec_tag = */ 0,
	/*ubtitle_codec = */ 0,
	/*metadata_conv = */ 0,
	/*next = */ 0
};
#endif
#endif

/*
 * RAW null muxer
 * Copyright (c) 2002 Fabrice Bellard
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

static int null_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
    return 0;
}

AVOutputFormat ff_null_muxer = {
    "null", /* name */
    NULL_IF_CONFIG_SMALL("raw null video"), /* long_name */
    0, /* mime_type */
    0, /* extensions */
    AV_NE(AV_CODEC_ID_PCM_S16BE, AV_CODEC_ID_PCM_S16LE), /* audio_codec */
    AV_CODEC_ID_RAWVIDEO, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOFILE | AVFMT_NOTIMESTAMPS | AVFMT_RAWPICTURE, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    null_write_packet, /* write_packet */
};

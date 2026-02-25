/*
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
#include "rawenc.h"

static int jacosub_write_header(AVFormatContext *s)
{
    const AVCodecContext *avctx = s->streams[0]->codec;

    if (avctx->extradata_size) {
        avio_write(s->pb, avctx->extradata, avctx->extradata_size - 1);
        avio_flush(s->pb);
    }
    return 0;
}

AVOutputFormat ff_jacosub_muxer = {
    "jacosub", /* name */
    NULL_IF_CONFIG_SMALL("JACOsub subtitle format"), /* long_name */
    "text/x-jacosub", /* mime_type */
    "jss,js", /* extensions */
    0, /* audio_codec */
    0, /* video_codec */
    AV_CODEC_ID_JACOSUB, /* subtitle_codec */
    AVFMT_TS_NONSTRICT, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    jacosub_write_header, /* write_header */
    ff_raw_write_packet, /* write_packet */
};

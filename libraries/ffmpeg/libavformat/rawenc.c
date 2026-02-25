/*
 * RAW muxers
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

#include "avformat.h"
#include "rawenc.h"

int ff_raw_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    avio_write(s->pb, pkt->data, pkt->size);
    avio_flush(s->pb);
    return 0;
}

/* Note: Do not forget to add new entries to the Makefile as well. */

#if CONFIG_AC3_MUXER
AVOutputFormat ff_ac3_muxer = {
    "ac3", /* name */
    NULL_IF_CONFIG_SMALL("raw AC-3"), /* long_name */
    "audio/x-ac3", /* mime_type */
    "ac3", /* extensions */
    AV_CODEC_ID_AC3, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_ADX_MUXER
AVOutputFormat ff_adx_muxer = {
    "adx", /* name */
    NULL_IF_CONFIG_SMALL("CRI ADX"), /* long_name */
    0, /* mime_type */
    "adx", /* extensions */
    AV_CODEC_ID_ADPCM_ADX, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_CAVSVIDEO_MUXER
AVOutputFormat ff_cavsvideo_muxer = {
    "cavsvideo", /* name */
    NULL_IF_CONFIG_SMALL("raw Chinese AVS (Audio Video Standard) video"), /* long_name */
    0, /* mime_type */
    "cavs", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_CAVS, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_DIRAC_MUXER
AVOutputFormat ff_dirac_muxer = {
    "dirac", /* name */
    NULL_IF_CONFIG_SMALL("raw Dirac"), /* long_name */
    0, /* mime_type */
    "drc", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_DIRAC, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_DNXHD_MUXER
AVOutputFormat ff_dnxhd_muxer = {
    "dnxhd", /* name */
    NULL_IF_CONFIG_SMALL("raw DNxHD (SMPTE VC-3)"), /* long_name */
    0, /* mime_type */
    "dnxhd", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_DNXHD, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_DTS_MUXER
AVOutputFormat ff_dts_muxer = {
    "dts", /* name */
    NULL_IF_CONFIG_SMALL("raw DTS"), /* long_name */
    "audio/x-dca", /* mime_type */
    "dts", /* extensions */
    AV_CODEC_ID_DTS, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_EAC3_MUXER
AVOutputFormat ff_eac3_muxer = {
    "eac3", /* name */
    NULL_IF_CONFIG_SMALL("raw E-AC-3"), /* long_name */
    "audio/x-eac3", /* mime_type */
    "eac3", /* extensions */
    AV_CODEC_ID_EAC3, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_G722_MUXER
AVOutputFormat ff_g722_muxer = {
    "g722", /* name */
    NULL_IF_CONFIG_SMALL("raw G.722"), /* long_name */
    "audio/G722", /* mime_type */
    "g722", /* extensions */
    AV_CODEC_ID_ADPCM_G722, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_G723_1_MUXER
AVOutputFormat ff_g723_1_muxer = {
    "g723_1", /* name */
    NULL_IF_CONFIG_SMALL("raw G.723.1"), /* long_name */
    "audio/g723", /* mime_type */
    "tco,rco", /* extensions */
    AV_CODEC_ID_G723_1, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_H261_MUXER
AVOutputFormat ff_h261_muxer = {
    "h261", /* name */
    NULL_IF_CONFIG_SMALL("raw H.261"), /* long_name */
    "video/x-h261", /* mime_type */
    "h261", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_H261, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_H263_MUXER
AVOutputFormat ff_h263_muxer = {
    "h263", /* name */
    NULL_IF_CONFIG_SMALL("raw H.263"), /* long_name */
    "video/x-h263", /* mime_type */
    "h263", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_H263, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_H264_MUXER
AVOutputFormat ff_h264_muxer = {
    "h264", /* name */
    NULL_IF_CONFIG_SMALL("raw H.264 video"), /* long_name */
    0, /* mime_type */
    "h264", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_H264, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_M4V_MUXER
AVOutputFormat ff_m4v_muxer = {
    "m4v", /* name */
    NULL_IF_CONFIG_SMALL("raw MPEG-4 video"), /* long_name */
    0, /* mime_type */
    "m4v", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_MPEG4, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_MJPEG_MUXER
AVOutputFormat ff_mjpeg_muxer = {
    "mjpeg", /* name */
    NULL_IF_CONFIG_SMALL("raw MJPEG video"), /* long_name */
    "video/x-mjpeg", /* mime_type */
    "mjpg,mjpeg", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_MJPEG, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_MLP_MUXER
AVOutputFormat ff_mlp_muxer = {
    "mlp", /* name */
    NULL_IF_CONFIG_SMALL("raw MLP"), /* long_name */
    0, /* mime_type */
    "mlp", /* extensions */
    AV_CODEC_ID_MLP, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_MPEG1VIDEO_MUXER
AVOutputFormat ff_mpeg1video_muxer = {
    "mpeg1video", /* name */
    NULL_IF_CONFIG_SMALL("raw MPEG-1 video"), /* long_name */
    "video/x-mpeg", /* mime_type */
    "mpg,mpeg,m1v", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_MPEG1VIDEO, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_MPEG2VIDEO_MUXER
AVOutputFormat ff_mpeg2video_muxer = {
    "mpeg2video", /* name */
    NULL_IF_CONFIG_SMALL("raw MPEG-2 video"), /* long_name */
    0, /* mime_type */
    "m2v", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_MPEG2VIDEO, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_RAWVIDEO_MUXER
AVOutputFormat ff_rawvideo_muxer = {
    "rawvideo", /* name */
    NULL_IF_CONFIG_SMALL("raw video"), /* long_name */
    0, /* mime_type */
    "yuv,rgb", /* extensions */
    AV_CODEC_ID_NONE, /* audio_codec */
    AV_CODEC_ID_RAWVIDEO, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

#if CONFIG_TRUEHD_MUXER
AVOutputFormat ff_truehd_muxer = {
    "truehd", /* name */
    NULL_IF_CONFIG_SMALL("raw TrueHD"), /* long_name */
    0, /* mime_type */
    "thd", /* extensions */
    AV_CODEC_ID_TRUEHD, /* audio_codec */
    AV_CODEC_ID_NONE, /* video_codec */
    0, /* subtitle_codec */
    AVFMT_NOTIMESTAMPS, /* flags */
    0, /* codec_tag */
    0, /* priv_class */
    0, /* next */
    0, /* priv_data_size */
    0, /* write_header */
    ff_raw_write_packet, /* write_packet */
};
#endif

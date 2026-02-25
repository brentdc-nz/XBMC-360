/*
 * Float MPEG Audio decoder
 * Copyright (c) 2010 Michael Niedermayer
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

#define CONFIG_FLOAT 1
#include "mpegaudiodec.c"

#if CONFIG_MP1FLOAT_DECODER
static const enum AVSampleFormat _ff_mpegaudiodec_float_fmts_105[] = { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_FLT,
                                                      AV_SAMPLE_FMT_NONE };
AVCodec ff_mp1float_decoder = {
        "mp1float", /* name */
        NULL_IF_CONFIG_SMALL("MP1 (MPEG audio layer 1)"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_MP1, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        _ff_mpegaudiodec_float_fmts_105, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(MPADecodeContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        decode_frame, /* decode */
        0, /* close */
        flush, /* flush */
    };
#endif
#if CONFIG_MP2FLOAT_DECODER
static const enum AVSampleFormat _ff_mpegaudiodec_float_fmts_106[] = { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_FLT,
                                                      AV_SAMPLE_FMT_NONE };
AVCodec ff_mp2float_decoder = {
        "mp2float", /* name */
        NULL_IF_CONFIG_SMALL("MP2 (MPEG audio layer 2)"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_MP2, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        _ff_mpegaudiodec_float_fmts_106, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(MPADecodeContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        decode_frame, /* decode */
        0, /* close */
        flush, /* flush */
    };
#endif
#if CONFIG_MP3FLOAT_DECODER
static const enum AVSampleFormat _ff_mpegaudiodec_float_fmts_107[] = { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_FLT,
                                                      AV_SAMPLE_FMT_NONE };
AVCodec ff_mp3float_decoder = {
        "mp3float", /* name */
        NULL_IF_CONFIG_SMALL("MP3 (MPEG audio layer 3)"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_MP3, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        _ff_mpegaudiodec_float_fmts_107, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(MPADecodeContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        decode_frame, /* decode */
        0, /* close */
        flush, /* flush */
    };
#endif
#if CONFIG_MP3ADUFLOAT_DECODER
static const enum AVSampleFormat _ff_mpegaudiodec_float_fmts_108[] = { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_FLT,
                                                      AV_SAMPLE_FMT_NONE };
AVCodec ff_mp3adufloat_decoder = {
        "mp3adufloat", /* name */
        NULL_IF_CONFIG_SMALL("ADU (Application Data Unit) MP3 (MPEG audio layer 3)"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_MP3ADU, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        _ff_mpegaudiodec_float_fmts_108, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(MPADecodeContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        decode_frame_adu, /* decode */
        0, /* close */
        flush, /* flush */
    };
#endif
#if CONFIG_MP3ON4FLOAT_DECODER
static const enum AVSampleFormat _ff_mpegaudiodec_float_fmts_109[] = { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_NONE };
AVCodec ff_mp3on4float_decoder = {
        "mp3on4float", /* name */
        NULL_IF_CONFIG_SMALL("MP3onMP4"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_MP3ON4, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        _ff_mpegaudiodec_float_fmts_109, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(MP3On4DecodeContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        decode_init_mp3on4, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        decode_frame_mp3on4, /* decode */
        decode_close_mp3on4, /* close */
        flush_mp3on4, /* flush */
    };
#endif

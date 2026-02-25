/*
 * Interface to libfaac for aac encoding
 * Copyright (c) 2002 Gildas Bazin <gbazin@netcourrier.com>
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
 * Interface to libfaac for aac encoding.
 */

#include <faac.h>

#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "avcodec.h"
#include "audio_frame_queue.h"
#include "internal.h"


/* libfaac has an encoder delay of 1024 samples */
#define FAAC_DELAY_SAMPLES 1024

typedef struct FaacAudioContext {
    faacEncHandle faac_handle;
    AudioFrameQueue afq;
} FaacAudioContext;

static av_cold int Faac_encode_close(AVCodecContext *avctx)
{
    FaacAudioContext *s = avctx->priv_data;

#if FF_API_OLD_ENCODE_AUDIO
    av_freep(&avctx->coded_frame);
#endif
    av_freep(&avctx->extradata);
    ff_af_queue_close(&s->afq);

    if (s->faac_handle)
        faacEncClose(s->faac_handle);

    return 0;
}

static const int channel_maps[][6] = {
    { 2, 0, 1 },          //< C L R
    { 2, 0, 1, 3 },       //< C L R Cs
    { 2, 0, 1, 3, 4 },    //< C L R Ls Rs
    { 2, 0, 1, 4, 5, 3 }, //< C L R Ls Rs LFE
};

static av_cold int Faac_encode_init(AVCodecContext *avctx)
{
    FaacAudioContext *s = avctx->priv_data;
    faacEncConfigurationPtr faac_cfg;
    unsigned long samples_input, max_bytes_output;
    int ret;

    /* number of channels */
    if (avctx->channels < 1 || avctx->channels > 6) {
        av_log(avctx, AV_LOG_ERROR, "encoding %d channel(s) is not allowed\n", avctx->channels);
        ret = AVERROR(EINVAL);
        goto error;
    }

    s->faac_handle = faacEncOpen(avctx->sample_rate,
                                 avctx->channels,
                                 &samples_input, &max_bytes_output);
    if (!s->faac_handle) {
        av_log(avctx, AV_LOG_ERROR, "error in faacEncOpen()\n");
        ret = AVERROR_UNKNOWN;
        goto error;
    }

    /* check faac version */
    faac_cfg = faacEncGetCurrentConfiguration(s->faac_handle);
    if (faac_cfg->version != FAAC_CFG_VERSION) {
        av_log(avctx, AV_LOG_ERROR, "wrong libfaac version (compiled for: %d, using %d)\n", FAAC_CFG_VERSION, faac_cfg->version);
        ret = AVERROR(EINVAL);
        goto error;
    }

    /* put the options in the configuration struct */
    switch(avctx->profile) {
        case FF_PROFILE_AAC_MAIN:
            faac_cfg->aacObjectType = MAIN;
            break;
        case FF_PROFILE_UNKNOWN:
        case FF_PROFILE_AAC_LOW:
            faac_cfg->aacObjectType = LOW;
            break;
        case FF_PROFILE_AAC_SSR:
            faac_cfg->aacObjectType = SSR;
            break;
        case FF_PROFILE_AAC_LTP:
            faac_cfg->aacObjectType = LTP;
            break;
        default:
            av_log(avctx, AV_LOG_ERROR, "invalid AAC profile\n");
            ret = AVERROR(EINVAL);
            goto error;
    }
    faac_cfg->mpegVersion = MPEG4;
    faac_cfg->useTns = 0;
    faac_cfg->allowMidside = 1;
    faac_cfg->bitRate = avctx->bit_rate / avctx->channels;
    faac_cfg->bandWidth = avctx->cutoff;
    if(avctx->flags & CODEC_FLAG_QSCALE) {
        faac_cfg->bitRate = 0;
        faac_cfg->quantqual = avctx->global_quality / FF_QP2LAMBDA;
    }
    faac_cfg->outputFormat = 1;
    faac_cfg->inputFormat = FAAC_INPUT_16BIT;
    if (avctx->channels > 2)
        memcpy(faac_cfg->channel_map, channel_maps[avctx->channels-3],
               avctx->channels * sizeof(int));

    avctx->frame_size = samples_input / avctx->channels;

#if FF_API_OLD_ENCODE_AUDIO
    avctx->coded_frame= avcodec_alloc_frame();
    if (!avctx->coded_frame) {
        ret = AVERROR(ENOMEM);
        goto error;
    }
#endif

    /* Set decoder specific info */
    avctx->extradata_size = 0;
    if (avctx->flags & CODEC_FLAG_GLOBAL_HEADER) {

        unsigned char *buffer = NULL;
        unsigned long decoder_specific_info_size;

        if (!faacEncGetDecoderSpecificInfo(s->faac_handle, &buffer,
                                           &decoder_specific_info_size)) {
            avctx->extradata = av_malloc(decoder_specific_info_size + FF_INPUT_BUFFER_PADDING_SIZE);
            if (!avctx->extradata) {
                ret = AVERROR(ENOMEM);
                goto error;
            }
            avctx->extradata_size = decoder_specific_info_size;
            memcpy(avctx->extradata, buffer, avctx->extradata_size);
            faac_cfg->outputFormat = 0;
        }
        free(buffer);
    }

    if (!faacEncSetConfiguration(s->faac_handle, faac_cfg)) {
        av_log(avctx, AV_LOG_ERROR, "libfaac doesn't support this output format!\n");
        ret = AVERROR(EINVAL);
        goto error;
    }

    avctx->delay = FAAC_DELAY_SAMPLES;
    ff_af_queue_init(avctx, &s->afq);

    return 0;
error:
    Faac_encode_close(avctx);
    return ret;
}

static int Faac_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
                             const AVFrame *frame, int *got_packet_ptr)
{
    FaacAudioContext *s = avctx->priv_data;
    int bytes_written, ret;
    int num_samples  = frame ? frame->nb_samples : 0;
    void *samples    = frame ? frame->data[0]    : NULL;

    if ((ret = ff_alloc_packet2(avctx, avpkt, (7 + 768) * avctx->channels)) < 0)
        return ret;

    bytes_written = faacEncEncode(s->faac_handle, samples,
                                  num_samples * avctx->channels,
                                  avpkt->data, avpkt->size);
    if (bytes_written < 0) {
        av_log(avctx, AV_LOG_ERROR, "faacEncEncode() error\n");
        return bytes_written;
    }

    /* add current frame to the queue */
    if (frame) {
        if ((ret = ff_af_queue_add(&s->afq, frame)) < 0)
            return ret;
    }

    if (!bytes_written)
        return 0;

    /* Get the next frame pts/duration */
    ff_af_queue_remove(&s->afq, avctx->frame_size, &avpkt->pts,
                       &avpkt->duration);

    avpkt->size = bytes_written;
    *got_packet_ptr = 1;
    return 0;
}

static const AVProfile profiles[] = {
    { FF_PROFILE_AAC_MAIN, "Main" },
    { FF_PROFILE_AAC_LOW,  "LC"   },
    { FF_PROFILE_AAC_SSR,  "SSR"  },
    { FF_PROFILE_AAC_LTP,  "LTP"  },
    { FF_PROFILE_UNKNOWN },
};

static const uint64_t faac_channel_layouts[] = {
    AV_CH_LAYOUT_MONO,
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_4POINT0,
    AV_CH_LAYOUT_5POINT0_BACK,
    AV_CH_LAYOUT_5POINT1_BACK,
    0
};

static const enum AVSampleFormat _ff_libfaac_fmts_66[] = { AV_SAMPLE_FMT_S16,
                                                     AV_SAMPLE_FMT_NONE };
AVCodec ff_libfaac_encoder = {
        "libfaac", /* name */
        NULL_IF_CONFIG_SMALL("libfaac AAC (Advanced Audio Coding)"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_AAC, /* id */
        CODEC_CAP_SMALL_LAST_FRAME | CODEC_CAP_DELAY, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        _ff_libfaac_fmts_66, /* sample_fmts */
        faac_channel_layouts, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        NULL_IF_CONFIG_SMALL(profiles), /* profiles */
        sizeof(FaacAudioContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        Faac_encode_init, /* init */
        0, /* encode_sub */
        Faac_encode_frame, /* encode2 */
        0, /* decode */
        Faac_encode_close, /* close */
    };

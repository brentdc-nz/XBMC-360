/*
 * iLBC decoder/encoder stub
 * Copyright (c) 2012 Martin Storsjo
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

#include <ilbc.h>

#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"

static int get_mode(AVCodecContext *avctx)
{
    if (avctx->block_align == 38)
        return 20;
    else if (avctx->block_align == 50)
        return 30;
    else if (avctx->bit_rate > 0)
        return avctx->bit_rate <= 14000 ? 30 : 20;
    else
        return -1;
}

typedef struct ILBCDecContext {
    const AVClass *class;
    iLBC_Dec_Inst_t decoder;
    int enhance;
} ILBCDecContext;

static const AVOption ilbc_dec_options[] = {
    { "enhance", "Enhance the decoded audio (adds delay)", offsetof(ILBCDecContext, enhance), AV_OPT_TYPE_INT, {0 }, 0, 1, AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_DECODING_PARAM },
    { NULL }
};

static const AVClass ilbc_dec_class = {
        "libilbc", /* class_name */
        av_default_item_name, /* item_name */
        ilbc_dec_options, /* option */
        LIBAVUTIL_VERSION_INT, /* version */
    };

static av_cold int ilbc_decode_init(AVCodecContext *avctx)
{
    ILBCDecContext *s  = avctx->priv_data;
    int mode;

    if ((mode = get_mode(avctx)) < 0) {
        av_log(avctx, AV_LOG_ERROR, "iLBC frame mode not indicated\n");
        return AVERROR(EINVAL);
    }

    WebRtcIlbcfix_InitDecode(&s->decoder, mode, s->enhance);

    avctx->channels       = 1;
    avctx->channel_layout = AV_CH_LAYOUT_MONO;
    avctx->sample_rate    = 8000;
    avctx->sample_fmt     = AV_SAMPLE_FMT_S16;

    return 0;
}

static int ilbc_decode_frame(AVCodecContext *avctx, void *data,
                             int *got_frame_ptr, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    ILBCDecContext *s  = avctx->priv_data;
    AVFrame *frame     = data;
    int ret;

    if (s->decoder.no_of_bytes > buf_size) {
        av_log(avctx, AV_LOG_ERROR, "iLBC frame too short (%u, should be %u)\n",
               buf_size, s->decoder.no_of_bytes);
        return AVERROR_INVALIDDATA;
    }

    frame->nb_samples = s->decoder.blockl;
    if ((ret = ff_get_buffer(avctx, frame)) < 0) {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return ret;
    }

    WebRtcIlbcfix_DecodeImpl((int16_t *) frame->data[0], (const uint16_t *) buf, &s->decoder, 1);

    *got_frame_ptr = 1;

    return s->decoder.no_of_bytes;
}

AVCodec ff_libilbc_decoder = {
        "libilbc", /* name */
        NULL_IF_CONFIG_SMALL("iLBC (Internet Low Bitrate Codec)"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_ILBC, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        &ilbc_dec_class, /* priv_class */
        0, /* profiles */
        sizeof(ILBCDecContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        ilbc_decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        ilbc_decode_frame, /* decode */
    };

typedef struct ILBCEncContext {
    const AVClass *class;
    iLBC_Enc_Inst_t encoder;
    int mode;
} ILBCEncContext;

static const AVOption ilbc_enc_options[] = {
    { "mode", "iLBC mode (20 or 30 ms frames)", offsetof(ILBCEncContext, mode), AV_OPT_TYPE_INT, {20 }, 20, 30, AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_ENCODING_PARAM },
    { NULL }
};

static const AVClass ilbc_enc_class = {
        "libilbc", /* class_name */
        av_default_item_name, /* item_name */
        ilbc_enc_options, /* option */
        LIBAVUTIL_VERSION_INT, /* version */
    };

static av_cold int ilbc_encode_init(AVCodecContext *avctx)
{
    ILBCEncContext *s = avctx->priv_data;
    int mode;

    if (avctx->sample_rate != 8000) {
        av_log(avctx, AV_LOG_ERROR, "Only 8000Hz sample rate supported\n");
        return AVERROR(EINVAL);
    }

    if (avctx->channels != 1) {
        av_log(avctx, AV_LOG_ERROR, "Only mono supported\n");
        return AVERROR(EINVAL);
    }

    if ((mode = get_mode(avctx)) > 0)
        s->mode = mode;
    else
        s->mode = s->mode != 30 ? 20 : 30;
    WebRtcIlbcfix_InitEncode(&s->encoder, s->mode);

    avctx->block_align = s->encoder.no_of_bytes;
    avctx->frame_size  = s->encoder.blockl;
#if FF_API_OLD_ENCODE_AUDIO
    avctx->coded_frame = avcodec_alloc_frame();
    if (!avctx->coded_frame)
        return AVERROR(ENOMEM);
#endif

    return 0;
}

static av_cold int ilbc_encode_close(AVCodecContext *avctx)
{
#if FF_API_OLD_ENCODE_AUDIO
    av_freep(&avctx->coded_frame);
#endif
    return 0;
}

static int ilbc_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
                             const AVFrame *frame, int *got_packet_ptr)
{
    ILBCEncContext *s = avctx->priv_data;
    int ret;

    if ((ret = ff_alloc_packet2(avctx, avpkt, 50)) < 0)
        return ret;

    WebRtcIlbcfix_EncodeImpl((uint16_t *) avpkt->data, (const int16_t *) frame->data[0], &s->encoder);

    avpkt->size     = s->encoder.no_of_bytes;
    *got_packet_ptr = 1;
    return 0;
}

static const AVCodecDefault ilbc_encode_defaults[] = {
    { "b", "0" },
    { NULL }
};

static const enum AVSampleFormat _ff_libilbc_fmts_70[] = { AV_SAMPLE_FMT_S16,
                                                     AV_SAMPLE_FMT_NONE };
AVCodec ff_libilbc_encoder = {
        "libilbc", /* name */
        NULL_IF_CONFIG_SMALL("iLBC (Internet Low Bitrate Codec)"), /* long_name */
        AVMEDIA_TYPE_AUDIO, /* type */
        AV_CODEC_ID_ILBC, /* id */
        0, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        _ff_libilbc_fmts_70, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        &ilbc_enc_class, /* priv_class */
        0, /* profiles */
        sizeof(ILBCEncContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        ilbc_encode_defaults, /* defaults */
        0, /* init_static_data */
        ilbc_encode_init, /* init */
        0, /* encode_sub */
        ilbc_encode_frame, /* encode2 */
        0, /* decode */
        ilbc_encode_close, /* close */
    };

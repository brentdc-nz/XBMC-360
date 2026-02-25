/*
 * Kega Game Video (KGV1) decoder
 * Copyright (c) 2010 Daniel Verkamp
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
 * Kega Game Video decoder
 */

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "internal.h"

typedef struct {
    AVCodecContext *avctx;
    AVFrame prev, cur;
} KgvContext;

static void decode_flush(AVCodecContext *avctx)
{
    KgvContext * const c = avctx->priv_data;

    if (c->prev.data[0])
        avctx->release_buffer(avctx, &c->prev);
}

static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
                        AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    const uint8_t *buf_end = buf + avpkt->size;
    KgvContext * const c = avctx->priv_data;
    int offsets[8];
    uint8_t *out, *prev;
    int outcnt = 0, maxcnt;
    int w, h, i, res;

    if (avpkt->size < 2)
        return AVERROR_INVALIDDATA;

    w = (buf[0] + 1) * 8;
    h = (buf[1] + 1) * 8;
    buf += 2;

    if ((res = av_image_check_size(w, h, 0, avctx)) < 0)
        return res;

    if (w != avctx->width || h != avctx->height) {
        if (c->prev.data[0])
            avctx->release_buffer(avctx, &c->prev);
        avcodec_set_dimensions(avctx, w, h);
    }

    maxcnt = w * h;

    c->cur.reference = 3;
    if ((res = ff_get_buffer(avctx, &c->cur)) < 0)
        return res;
    out  = c->cur.data[0];
    if (c->prev.data[0]) {
        prev = c->prev.data[0];
    } else {
        prev = NULL;
    }

    for (i = 0; i < 8; i++)
        offsets[i] = -1;

    while (outcnt < maxcnt && buf_end - 2 >= buf) {
        int code = AV_RL16(buf);
        buf += 2;

        if (!(code & 0x8000)) {
            AV_WN16A(&out[2 * outcnt], code); // rgb555 pixel coded directly
            outcnt++;
        } else {
            int count;

            if ((code & 0x6000) == 0x6000) {
                // copy from previous frame
                int oidx = (code >> 10) & 7;
                int start;

                count = (code & 0x3FF) + 3;

                if (offsets[oidx] < 0) {
                    if (buf_end - 3 < buf)
                        break;
                    offsets[oidx] = AV_RL24(buf);
                    buf += 3;
                }

                start = (outcnt + offsets[oidx]) % maxcnt;

                if (maxcnt - start < count || maxcnt - outcnt < count)
                    break;

                if (!prev) {
                    av_log(avctx, AV_LOG_ERROR,
                           "Frame reference does not exist\n");
                    break;
                }

                memcpy(out + 2 * outcnt, prev + 2 * start, 2 * count);
            } else {
                // copy from earlier in this frame
                int offset = (code & 0x1FFF) + 1;

                if (!(code & 0x6000)) {
                    count = 2;
                } else if ((code & 0x6000) == 0x2000) {
                    count = 3;
                } else {
                    if (buf_end - 1 < buf)
                        break;
                    count = 4 + *buf++;
                }

                if (outcnt < offset || maxcnt - outcnt < count)
                    break;

                av_memcpy_backptr(out + 2 * outcnt, 2 * offset, 2 * count);
            }
            outcnt += count;
        }
    }

    if (outcnt - maxcnt)
        av_log(avctx, AV_LOG_DEBUG, "frame finished with %d diff\n", outcnt - maxcnt);

    *got_frame = 1;
    *(AVFrame*)data = c->cur;

    if (c->prev.data[0])
        avctx->release_buffer(avctx, &c->prev);
    FFSWAP(AVFrame, c->cur, c->prev);

    return avpkt->size;
}

static av_cold int decode_init(AVCodecContext *avctx)
{
    KgvContext * const c = avctx->priv_data;

    c->avctx = avctx;
    avctx->pix_fmt = AV_PIX_FMT_RGB555;
    avctx->flags  |= CODEC_FLAG_EMU_EDGE;

    return 0;
}

static av_cold int decode_end(AVCodecContext *avctx)
{
    decode_flush(avctx);
    return 0;
}

AVCodec ff_kgv1_decoder = {
        "kgv1", /* name */
        NULL_IF_CONFIG_SMALL("Kega Game Video"), /* long_name */
        AVMEDIA_TYPE_VIDEO, /* type */
        AV_CODEC_ID_KGV1, /* id */
        CODEC_CAP_DR1, /* capabilities */
        0, /* supported_framerates */
        0, /* pix_fmts */
        0, /* supported_samplerates */
        0, /* sample_fmts */
        0, /* channel_layouts */
        0, /* max_lowres */
        0, /* priv_class */
        0, /* profiles */
        sizeof(KgvContext), /* priv_data_size */
        0, /* next */
        0, /* init_thread_copy */
        0, /* update_thread_context */
        0, /* defaults */
        0, /* init_static_data */
        decode_init, /* init */
        0, /* encode_sub */
        0, /* encode2 */
        decode_frame, /* decode */
        decode_end, /* close */
        decode_flush, /* flush */
    };

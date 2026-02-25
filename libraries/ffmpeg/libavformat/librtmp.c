/*
 * RTMP network protocol
 * Copyright (c) 2010 Howard Chu
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
 * RTMP protocol based on http://rtmpdump.mplayerhq.hu/ librtmp
 */

#include "libavutil/avstring.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include "url.h"

#include <librtmp/rtmp.h>
#include <librtmp/log.h>

typedef struct LibRTMPContext {
    const AVClass *class;
    RTMP rtmp;
    char *app;
    char *playpath;
} LibRTMPContext;

static void rtmp_log(int level, const char *fmt, va_list args)
{
    switch (level) {
    default:
    case RTMP_LOGCRIT:    level = AV_LOG_FATAL;   break;
    case RTMP_LOGERROR:   level = AV_LOG_ERROR;   break;
    case RTMP_LOGWARNING: level = AV_LOG_WARNING; break;
    case RTMP_LOGINFO:    level = AV_LOG_INFO;    break;
    case RTMP_LOGDEBUG:   level = AV_LOG_VERBOSE; break;
    case RTMP_LOGDEBUG2:  level = AV_LOG_DEBUG;   break;
    }

    av_vlog(NULL, level, fmt, args);
    av_log(NULL, level, "\n");
}

static int rtmp_close(URLContext *s)
{
    LibRTMPContext *ctx = s->priv_data;
    RTMP *r = &ctx->rtmp;

    RTMP_Close(r);
    return 0;
}

/**
 * Open RTMP connection and verify that the stream can be played.
 *
 * URL syntax: rtmp://server[:port][/app][/playpath][ keyword=value]...
 *             where 'app' is first one or two directories in the path
 *             (e.g. /ondemand/, /flash/live/, etc.)
 *             and 'playpath' is a file name (the rest of the path,
 *             may be prefixed with "mp4:")
 *
 *             Additional RTMP library options may be appended as
 *             space-separated key-value pairs.
 */
static int rtmp_open(URLContext *s, const char *uri, int flags)
{
    LibRTMPContext *ctx = s->priv_data;
    RTMP *r = &ctx->rtmp;
    int rc = 0, level;
    char *filename = s->filename;

    switch (av_log_get_level()) {
    default:
    case AV_LOG_FATAL:   level = RTMP_LOGCRIT;    break;
    case AV_LOG_ERROR:   level = RTMP_LOGERROR;   break;
    case AV_LOG_WARNING: level = RTMP_LOGWARNING; break;
    case AV_LOG_INFO:    level = RTMP_LOGINFO;    break;
    case AV_LOG_VERBOSE: level = RTMP_LOGDEBUG;   break;
    case AV_LOG_DEBUG:   level = RTMP_LOGDEBUG2;  break;
    }
    RTMP_LogSetLevel(level);
    RTMP_LogSetCallback(rtmp_log);

    if (ctx->app || ctx->playpath) {
        int len = strlen(s->filename) + 1;
        if (ctx->app)      len += strlen(ctx->app)      + sizeof(" app=");
        if (ctx->playpath) len += strlen(ctx->playpath) + sizeof(" playpath=");

        if (!(filename = av_malloc(len)))
            return AVERROR(ENOMEM);

        av_strlcpy(filename, s->filename, len);
        if (ctx->app) {
            av_strlcat(filename, " app=", len);
            av_strlcat(filename, ctx->app, len);
        }
        if (ctx->playpath) {
            av_strlcat(filename, " playpath=", len);
            av_strlcat(filename, ctx->playpath, len);
        }
    }

    RTMP_Init(r);
    if (!RTMP_SetupURL(r, filename)) {
        rc = AVERROR_UNKNOWN;
        goto fail;
    }

    if (flags & AVIO_FLAG_WRITE)
        RTMP_EnableWrite(r);

    if (!RTMP_Connect(r, NULL) || !RTMP_ConnectStream(r, 0)) {
        rc = AVERROR_UNKNOWN;
        goto fail;
    }

    s->is_streamed = 1;
    rc = 0;
fail:
    if (filename != s->filename)
        av_freep(&filename);
    return rc;
}

static int rtmp_write(URLContext *s, const uint8_t *buf, int size)
{
    LibRTMPContext *ctx = s->priv_data;
    RTMP *r = &ctx->rtmp;

    return RTMP_Write(r, buf, size);
}

static int rtmp_read(URLContext *s, uint8_t *buf, int size)
{
    LibRTMPContext *ctx = s->priv_data;
    RTMP *r = &ctx->rtmp;

    return RTMP_Read(r, buf, size);
}

static int rtmp_read_pause(URLContext *s, int pause)
{
    LibRTMPContext *ctx = s->priv_data;
    RTMP *r = &ctx->rtmp;

    if (!RTMP_Pause(r, pause))
        return AVERROR_UNKNOWN;
    return 0;
}

static int64_t rtmp_read_seek(URLContext *s, int stream_index,
                              int64_t timestamp, int flags)
{
    LibRTMPContext *ctx = s->priv_data;
    RTMP *r = &ctx->rtmp;

    if (flags & AVSEEK_FLAG_BYTE)
        return AVERROR(ENOSYS);

    /* seeks are in milliseconds */
    if (stream_index < 0)
        timestamp = av_rescale_rnd(timestamp, 1000, AV_TIME_BASE,
            flags & AVSEEK_FLAG_BACKWARD ? AV_ROUND_DOWN : AV_ROUND_UP);

    if (!RTMP_SendSeek(r, timestamp))
        return AVERROR_UNKNOWN;
    return timestamp;
}

static int rtmp_get_file_handle(URLContext *s)
{
    LibRTMPContext *ctx = s->priv_data;
    RTMP *r = &ctx->rtmp;

    return RTMP_Socket(r);
}

#define OFFSET(x) offsetof(LibRTMPContext, x)
#define DEC AV_OPT_FLAG_DECODING_PARAM
#define ENC AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
    {"rtmp_app",      "Name of application to connect to on the RTMP server", OFFSET(app),      AV_OPT_TYPE_STRING, { 0, 0, NULL }, 0, 0, DEC|ENC},
    {"rtmp_playpath", "Stream identifier to play or to publish",              OFFSET(playpath), AV_OPT_TYPE_STRING, { 0, 0, NULL }, 0, 0, DEC|ENC},
    { NULL },
};

#define RTMP_CLASS(flavor)\
static const AVClass lib ## flavor ## _class = {\
    /* class_name */ "lib" #flavor " protocol",\
    /* item_name */ av_default_item_name,\
    /* option */ options,\
    /* version */ LIBAVUTIL_VERSION_INT,\
};

RTMP_CLASS(rtmp)
URLProtocol ff_librtmp_protocol = {
    "rtmp", /* name */
    rtmp_open, /* url_open */
    rtmp_read, /* url_read */
    rtmp_write, /* url_write */
    0, /* url_seek */
    rtmp_close, /* url_close */
    0, /* next */
    rtmp_read_pause, /* url_read_pause */
    rtmp_read_seek, /* url_read_seek */
    rtmp_get_file_handle, /* url_get_file_handle */
    0, /* url_get_multi_file_handle */
    0, /* url_shutdown */
    sizeof(LibRTMPContext), /* priv_data_size */
    &librtmp_class, /* priv_data_class */
    URL_PROTOCOL_FLAG_NETWORK, /* flags */
};

RTMP_CLASS(rtmpt)
URLProtocol ff_librtmpt_protocol = {
    "rtmpt", /* name */
    rtmp_open, /* url_open */
    rtmp_read, /* url_read */
    rtmp_write, /* url_write */
    0, /* url_seek */
    rtmp_close, /* url_close */
    0, /* next */
    rtmp_read_pause, /* url_read_pause */
    rtmp_read_seek, /* url_read_seek */
    rtmp_get_file_handle, /* url_get_file_handle */
    0, /* url_get_multi_file_handle */
    0, /* url_shutdown */
    sizeof(LibRTMPContext), /* priv_data_size */
    &librtmpt_class, /* priv_data_class */
    URL_PROTOCOL_FLAG_NETWORK, /* flags */
};

RTMP_CLASS(rtmpe)
URLProtocol ff_librtmpe_protocol = {
    "rtmpe", /* name */
    rtmp_open, /* url_open */
    rtmp_read, /* url_read */
    rtmp_write, /* url_write */
    0, /* url_seek */
    rtmp_close, /* url_close */
    0, /* next */
    rtmp_read_pause, /* url_read_pause */
    rtmp_read_seek, /* url_read_seek */
    rtmp_get_file_handle, /* url_get_file_handle */
    0, /* url_get_multi_file_handle */
    0, /* url_shutdown */
    sizeof(LibRTMPContext), /* priv_data_size */
    &librtmpe_class, /* priv_data_class */
    URL_PROTOCOL_FLAG_NETWORK, /* flags */
};

RTMP_CLASS(rtmpte)
URLProtocol ff_librtmpte_protocol = {
    "rtmpte", /* name */
    rtmp_open, /* url_open */
    rtmp_read, /* url_read */
    rtmp_write, /* url_write */
    0, /* url_seek */
    rtmp_close, /* url_close */
    0, /* next */
    rtmp_read_pause, /* url_read_pause */
    rtmp_read_seek, /* url_read_seek */
    rtmp_get_file_handle, /* url_get_file_handle */
    0, /* url_get_multi_file_handle */
    0, /* url_shutdown */
    sizeof(LibRTMPContext), /* priv_data_size */
    &librtmpte_class, /* priv_data_class */
    URL_PROTOCOL_FLAG_NETWORK, /* flags */
};

RTMP_CLASS(rtmps)
URLProtocol ff_librtmps_protocol = {
    "rtmps", /* name */
    rtmp_open, /* url_open */
    rtmp_read, /* url_read */
    rtmp_write, /* url_write */
    0, /* url_seek */
    rtmp_close, /* url_close */
    0, /* next */
    rtmp_read_pause, /* url_read_pause */
    rtmp_read_seek, /* url_read_seek */
    rtmp_get_file_handle, /* url_get_file_handle */
    0, /* url_get_multi_file_handle */
    0, /* url_shutdown */
    sizeof(LibRTMPContext), /* priv_data_size */
    &librtmps_class, /* priv_data_class */
    URL_PROTOCOL_FLAG_NETWORK, /* flags */
};

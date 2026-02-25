/*
 * Buffered file io for ffmpeg system - Xbox 360 SMB file protocol
 * Copyright (c) 2001 Fabrice Bellard
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
 * Xbox 360 SMB file protocol handler
 * Adapted for FFmpeg 1.2 URLProtocol API
 */

#ifdef _XBOX

#include "avformat.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "os_support.h"
#include "url.h"

/*
 * Interface to SMB files on Xbox 360
 * These are the exports for SMB access provided by the XBMC application layer
 */

extern int XBFS_open(const char* szFileName, int iMode);
extern int XBFS_read(int fd, void* buffer, unsigned int uiSize);
extern int XBFS_write(int fd, const void* buffer, unsigned int uiSize);
extern long XBFS_lseek(int fd, long lPos, int iWhence);
extern int XBFS_fstat(int fd, struct stat* buffer);
extern int XBFS_stat(const char *path, struct _stat *buffer);
extern int XBFS_close(int fd);
extern int XBFS_fclose(FILE * stream);

/* SMB file protocol callbacks */

static int file_read(URLContext *h, unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    return XBFS_read(fd, buf, size);
}

static int file_write(URLContext *h, const unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    return XBFS_write(fd, buf, size);
}

static int file_get_handle(URLContext *h)
{
    return (intptr_t) h->priv_data;
}

static int file_check(URLContext *h, int mask)
{
    struct stat st;
    int ret = stat(h->filename, &st);

    if (ret < 0)
        return AVERROR(errno);

    return ret;
}

#if CONFIG_SMBFILE_PROTOCOL

static int file_open(URLContext *h, const char *filename, int flags)
{
    int access;
    int fd;

    if (flags & AVIO_FLAG_READ_WRITE)
        access = O_CREAT | O_TRUNC | O_RDWR;
    else if (flags & AVIO_FLAG_WRITE)
        access = O_CREAT | O_TRUNC | O_WRONLY;
    else
        access = O_RDONLY;

#ifdef O_BINARY
    access |= O_BINARY;
#endif

    fd = XBFS_open(filename, access);

    if (fd == -1)
        return AVERROR(errno);

    h->priv_data = (void *) (intptr_t) fd;

    return 0;
}

/* XXX: use llseek */
static int64_t file_seek(URLContext *h, int64_t pos, int whence)
{
    int fd = (intptr_t) h->priv_data;

    if (whence == AVSEEK_SIZE)
    {
        struct stat st;
        int ret = XBFS_fstat(fd, &st);
        return ret < 0 ? AVERROR(errno) : st.st_size;
    }

    return XBFS_lseek(fd, pos, whence);
}

static int file_close(URLContext *h)
{
    int fd = (intptr_t) h->priv_data;
    return XBFS_close(fd);
}

/*
 * FFmpeg 1.2 URLProtocol struct has the following fields:
 *   name, url_open, url_open2, url_read, url_write, url_seek, url_close,
 *   next, url_read_pause, url_read_seek, url_get_file_handle,
 *   url_get_multi_file_handle, url_shutdown, priv_data_size,
 *   priv_data_class, flags, url_check
 *
 * We use MSC_STRUCTS positional initialization for MSVC compatibility.
 */
URLProtocol ff_smbfile_protocol = {
    "smb", /* name */
    file_open, /* url_open */
    0, /* url_open2 */
    file_read, /* url_read */
    file_write, /* url_write */
    file_seek, /* url_seek */
    file_close, /* url_close */
    0, /* next */
    0, /* url_read_pause */
    0, /* url_read_seek */
    file_get_handle, /* url_get_file_handle */
    0, /* url_get_multi_file_handle */
    0, /* url_shutdown */
    0, /* priv_data_size */
    0, /* priv_data_class */
    0, /* flags */
    file_check, /* url_check */
};

#endif /* CONFIG_SMBFILE_PROTOCOL */

#endif /* _XBOX */

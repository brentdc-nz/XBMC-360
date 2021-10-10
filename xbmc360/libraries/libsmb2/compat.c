/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
/*
   Copyright (C) 2020 by Ronnie Sahlberg <ronniesahlberg@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "compat.h"

#ifdef ESP_PLATFORM

#define NEED_READV
#define NEED_WRITEV

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <lwip/sockets.h>
#include <sys/uio.h>

#endif

#ifdef PS2_EE_PLATFORM

#define NEED_READV
#define NEED_WRITEV
#define NEED_POLL
#define NEED_BE64TOH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getaddrinfo(const char *node, const char*service,
                const struct addrinfo *hints,
                struct addrinfo **res)
{
        struct sockaddr_in *sin;
        struct hostent *host;
        int i, ip[4];

        sin = malloc(sizeof(struct sockaddr_in));
        sin->sin_len = sizeof(struct sockaddr_in);
        sin->sin_family=AF_INET;

        /* Some error checking would be nice */
        if (sscanf(node, "%d.%d.%d.%d", ip, ip+1, ip+2, ip+3) == 4) {
                for (i = 0; i < 4; i++) {
                        ((char *)&sin->sin_addr.s_addr)[i] = ip[i];
                }
        } else {
                host = gethostbyname(node);
                if (host == NULL) {
                        return -1;
                }
                if (host->h_addrtype != AF_INET) {
                        return -2;
                }
                memcpy(&sin->sin_addr.s_addr, host->h_addr, 4);
        }

        sin->sin_port=0;
        if (service) {
                sin->sin_port=htons(atoi(service));
        } 

        *res = malloc(sizeof(struct addrinfo));

        (*res)->ai_family = AF_INET;
        (*res)->ai_addrlen = sizeof(struct sockaddr_in);
        (*res)->ai_addr = (struct sockaddr *)sin;

        return 0;
}

void freeaddrinfo(struct addrinfo *res)
{
        free(res->ai_addr);
        free(res);
}

#endif /* PS2_EE_PLATFORM */

#ifdef PS2_IOP_PLATFORM
#include <sysclib.h>

#define NEED_BE64TOH
#define NEED_STRDUP
#define NEED_READV
#define NEED_WRITEV
#define NEED_POLL

static unsigned long int next = 1; 

int random(void)
{ 
    next = next * 1103515245 + 12345; 
    return (unsigned int)(next/65536) % 32768; 
} 

void srandom(unsigned int seed) 
{ 
    next = seed; 
}

#include <thbase.h>
time_t time(time_t *tloc)
{
        u32 sec, usec;
        iop_sys_clock_t sys_clock;

        GetSystemTime(&sys_clock);
        SysClock2USec(&sys_clock, &sec, &usec);

        return sec;
}

#include <stdio.h>
#include <stdarg.h>
int asprintf(char **strp, const char *fmt, ...)
{
        int len;
        char *str;
        va_list args;        

        va_start(args, fmt);
        str = malloc(256);
        len = sprintf(str, fmt, args);
        va_end(args);
        *strp = str;
        return len;
}

int errno;

int iop_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
        int rc;
        int err = 0;
        socklen_t err_size = sizeof(err);

        if ((rc = lwip_connect(sockfd, addr, addrlen)) < 0) {
                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
			       (char *)&err, &err_size) != 0 || err != 0) {
                        errno = err;
                }
        }

        return rc;
}

#endif /* PS2_IOP_PLATFORM */

#ifdef PS3_PPU_PLATFORM

#define NEED_READV
#define NEED_WRITEV

#include <stdlib.h>

int smb2_getaddrinfo(const char *node, const char*service,
                const struct addrinfo *hints,
                struct addrinfo **res)
{
        struct sockaddr_in *sin;

        sin = malloc(sizeof(struct sockaddr_in));
        sin->sin_len = sizeof(struct sockaddr_in);
        sin->sin_family=AF_INET;

        /* Some error checking would be nice */
        sin->sin_addr.s_addr = inet_addr(node);

        sin->sin_port=0;
        if (service) {
                sin->sin_port=htons(atoi(service));
        } 

        *res = malloc(sizeof(struct addrinfo));

        (*res)->ai_family = AF_INET;
        (*res)->ai_addrlen = sizeof(struct sockaddr_in);
        (*res)->ai_addr = (struct sockaddr *)sin;

        return 0;
}

void smb2_freeaddrinfo(struct addrinfo *res)
{
        free(res->ai_addr);
        free(res);
}

#endif /* PS3_PPU_PLATFORM */

#ifdef NEED_WRITEV
ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
        int i, count, left, total = 0;
        char *ptr;

        for (i = 0; i < iovcnt; i++) {
                left = iov[i].iov_len;
                ptr = iov[i].iov_base;
                while (left > 0) {
                        count = write(fd, ptr, left);
                        if (count == -1) {
                                return -1;
                        }
                        total += count;
                        left -= count;
                        ptr += count;
                }
        }
        return total;
}
#endif

#ifdef NEED_READV
ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
        int i, left, count;
        ssize_t total = 0;
        char *ptr;

        for (i = 0; i < iovcnt; i++) {
                left = iov[i].iov_len;
                ptr = iov[i].iov_base;
                while (left > 0) {
                        count = read(fd, ptr, left);
                        if (count == -1) {
                                return -1;
                        }
                        if (count == 0) {
                                return total;
                        }
                        total += count;
                        left -= count;
                        ptr += count;
                }
        }
        return total;
}
#endif

#ifdef _XBOX //NEED_POLL
int poll(struct pollfd *fds, nfds_t numfds, int timeout)
{
    fd_set read_set;
    fd_set write_set;
    fd_set exception_set;
    nfds_t i;
    int n;
    int rc;

#if 0
    if (numfds >= FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }
#endif

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&exception_set);

    n = 0;
    for (i = 0; i < numfds; i++) {
        if (fds[i].fd < 0)
            continue;
#if 0
        if (fds[i].fd >= FD_SETSIZE) {
            errno = EINVAL;
            return -1;
        }
#endif

        if (fds[i].events & POLLIN)
            FD_SET(fds[i].fd, &read_set);
        if (fds[i].events & POLLOUT)
            FD_SET(fds[i].fd, &write_set);
        if (fds[i].events & POLLERR)
            FD_SET(fds[i].fd, &exception_set);

        if (fds[i].fd >= n)
            n = fds[i].fd + 1;
    }

    if (n == 0)
        /* Hey!? Nothing to poll, in fact!!! */
        return 0;

    if (timeout < 0) {
        rc = select(n, &read_set, &write_set, &exception_set, NULL);
    } else {
        struct timeval tv;
        tv.tv_sec  = timeout / 1000;
        tv.tv_usec = 1000 * (timeout % 1000);
        rc         = select(n, &read_set, &write_set, &exception_set, &tv);
    }

    if (rc < 0)
        return rc;

    for (i = 0; i < numfds; i++) {
        fds[i].revents = 0;

        if (FD_ISSET(fds[i].fd, &read_set))
            fds[i].revents |= POLLIN;
        if (FD_ISSET(fds[i].fd, &write_set))
            fds[i].revents |= POLLOUT;
        if (FD_ISSET(fds[i].fd, &exception_set))
            fds[i].revents |= POLLERR;
    }

    return rc;
}
#endif

#ifdef NEED_STRDUP
char *strdup(const char *s)
{
        char *str;
        int len;

        len = strlen(s) + 1;
        str = malloc(len);
        if (str == NULL) {
#ifndef PS2_IOP_PLATFORM
                errno = ENOMEM;
#endif /* !PS2_IOP_PLATFORM */
                return NULL;
        }
        memcpy(str, s, len + 1);
        return str;
}
#endif /* NEED_STRDUP */

#ifdef NEED_BE64TOH
long long int be64toh(long long int x)
{
  long long int val;

  val = ntohl(x & 0xffffffff);
  val <<= 32;
  val ^= ntohl(x >> 32);
  return val;
}
#endif /* NEED_BE64TOH */

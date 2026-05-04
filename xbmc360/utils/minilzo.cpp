/*
 * miniLZO -- mini subset of the LZO real-time data compression library
 * http://www.oberhumer.com/opensource/lzo/
 *
 * LZO1X-1 decompressor with bounds checking (safe variant).
 * Extracted from LZO 2.x for Xbox 360 runtime use.
 *
 * Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
 * Licensed under the GNU General Public License (GPL v2+)
 *
 * NOTE: This is a minimal implementation sufficient for decompressing
 * data compressed by lzo1x_999_compress() in the build tool.
 * For the full LZO library, see http://www.oberhumer.com/opensource/lzo/
 */

#include "minilzo.h"

#include <string.h>

/* internal macros */
#define LZO_BYTE(x)       ((unsigned char)(x))

/* LZO stream stores multi-byte values in little-endian order.
 * Must read byte-by-byte on big-endian platforms (Xbox 360 PowerPC). */
static inline unsigned short lzo_get_le16(const unsigned char* p)
{
    return (unsigned short)p[0] | ((unsigned short)p[1] << 8);
}
#define UA_GET16(p)        lzo_get_le16((const unsigned char*)(p))

#define M2_MAX_OFFSET   0x0800
#define M3_MAX_OFFSET   0x4000
#define M4_MAX_OFFSET   0xbfff

#define M1_MAX_LEN      2
#define M2_MAX_LEN      8
#define M3_MAX_LEN      33
#define M4_MAX_LEN      9

#define M1_MARKER       0
#define M2_MARKER       64
#define M3_MARKER       32
#define M4_MARKER       16

int lzo_init(void)
{
    return LZO_E_OK;
}

int lzo1x_decompress_safe(const lzo_byte* in, lzo_uint in_len,
                          lzo_byte* out, lzo_uint* out_len,
                          void* wrkmem)
{
    lzo_byte* op;
    const lzo_byte* ip;
    lzo_uint t;
    const lzo_byte* m_pos;
    const lzo_byte* const ip_end = in + in_len;
    lzo_byte* const op_end = out + *out_len;

    (void)wrkmem;

    *out_len = 0;

    op = out;
    ip = in;

    if (*ip > 17)
    {
        t = *ip++ - 17;
        if (t < 4)
            goto match_next;
        if ((op_end - op) < (int)t || (ip_end - ip) < (int)t + 1)
            return LZO_E_INPUT_OVERRUN;
        do *op++ = *ip++; while (--t > 0);
        goto first_literal_run;
    }

    for (;;)
    {
        t = *ip++;
        if (t >= 16)
            goto match;

        /* a literal run */
        if (t == 0)
        {
            if (ip >= ip_end)
                return LZO_E_INPUT_OVERRUN;
            while (*ip == 0)
            {
                t += 255;
                ip++;
                if (ip >= ip_end)
                    return LZO_E_INPUT_OVERRUN;
            }
            t += 15 + *ip++;
        }
        /* copy literals */
        t += 3;
        if ((op_end - op) < (int)t || (ip_end - ip) < (int)t + 1)
            return LZO_E_INPUT_OVERRUN;
        memcpy(op, ip, t);
        op += t;
        ip += t;

first_literal_run:
        t = *ip++;
        if (t >= 16)
            goto match;

        m_pos = op - (1 + M2_MAX_OFFSET);
        m_pos -= t >> 2;
        m_pos -= *ip++ << 2;
        if (m_pos < out || m_pos >= op)
            return LZO_E_LOOKBEHIND_OVERRUN;
        if ((op_end - op) < 3)
            return LZO_E_OUTPUT_OVERRUN;
        *op++ = *m_pos++;
        *op++ = *m_pos++;
        *op++ = *m_pos;
        goto match_done;

        for (;;)
        {
match:
            if (t >= 64)                /* a M2 match */
            {
                m_pos = op - 1;
                m_pos -= (t >> 2) & 7;
                m_pos -= *ip++ << 3;
                t = (t >> 5) - 1;
                if (m_pos < out || m_pos >= op)
                    return LZO_E_LOOKBEHIND_OVERRUN;
                if ((op_end - op) < (int)t + 3 - 1)
                    return LZO_E_OUTPUT_OVERRUN;
                goto copy_match;
            }
            else if (t >= 32)           /* a M3 match */
            {
                t &= 31;
                if (t == 0)
                {
                    if (ip >= ip_end)
                        return LZO_E_INPUT_OVERRUN;
                    while (*ip == 0)
                    {
                        t += 255;
                        ip++;
                        if (ip >= ip_end)
                            return LZO_E_INPUT_OVERRUN;
                    }
                    t += 31 + *ip++;
                }
                m_pos = op - 1;
                {
                    unsigned short w = UA_GET16(ip);
                    ip += 2;
                    m_pos -= (w >> 2);
                }
            }
            else if (t >= 16)           /* a M4 match */
            {
                m_pos = op;
                m_pos -= (t & 8) << 11;
                t &= 7;
                if (t == 0)
                {
                    if (ip >= ip_end)
                        return LZO_E_INPUT_OVERRUN;
                    while (*ip == 0)
                    {
                        t += 255;
                        ip++;
                        if (ip >= ip_end)
                            return LZO_E_INPUT_OVERRUN;
                    }
                    t += 7 + *ip++;
                }
                {
                    unsigned short w = UA_GET16(ip);
                    ip += 2;
                    m_pos -= (w >> 2);
                }
                if (m_pos == op)
                    goto eof_found;
                m_pos -= 0x4000;
            }
            else                        /* a M1 match */
            {
                m_pos = op - 1;
                m_pos -= t >> 2;
                m_pos -= *ip++ << 2;
                if (m_pos < out || m_pos >= op)
                    return LZO_E_LOOKBEHIND_OVERRUN;
                if ((op_end - op) < 2)
                    return LZO_E_OUTPUT_OVERRUN;
                *op++ = *m_pos++;
                *op++ = *m_pos;
                goto match_done;
            }

            /* validate match */
            if (m_pos < out || m_pos >= op)
                return LZO_E_LOOKBEHIND_OVERRUN;
            if ((op_end - op) < (int)t + 3 - 1)
                return LZO_E_OUTPUT_OVERRUN;

copy_match:
            *op++ = *m_pos++;
            *op++ = *m_pos++;
            do *op++ = *m_pos++; while (--t > 0);

match_done:
            t = ip[-2] & 3;
            if (t == 0)
                break;

match_next:
            if ((op_end - op) < (int)t || (ip_end - ip) < (int)t + 1)
                return LZO_E_INPUT_OVERRUN;
            *op++ = *ip++;
            if (t > 1) { *op++ = *ip++; if (t > 2) { *op++ = *ip++; } }
            t = *ip++;
        }
    }

eof_found:
    *out_len = (lzo_uint)(op - out);
    if (ip < ip_end)
        return LZO_E_INPUT_NOT_CONSUMED;
    if (ip > ip_end)
        return LZO_E_INPUT_OVERRUN;
    return (ip == ip_end) ? LZO_E_OK : LZO_E_ERROR;
}

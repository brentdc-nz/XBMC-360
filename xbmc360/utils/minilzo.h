/*
 * miniLZO -- mini subset of the LZO real-time data compression library
 * http://www.oberhumer.com/opensource/lzo/
 *
 * This is a minimal self-contained LZO1X-1 decompressor extracted from LZO 2.x.
 * Only lzo1x_decompress_safe() is included - sufficient for XPR bundle loading.
 *
 * Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
 * Licensed under the GNU General Public License (GPL v2+)
 */

#ifndef __MINILZO_H
#define __MINILZO_H

#define MINILZO_VERSION         0x2100

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int lzo_uint;
typedef unsigned char lzo_byte;

#define LZO_E_OK                    0
#define LZO_E_ERROR                 (-1)
#define LZO_E_INPUT_OVERRUN         (-4)
#define LZO_E_OUTPUT_OVERRUN        (-5)
#define LZO_E_LOOKBEHIND_OVERRUN    (-6)
#define LZO_E_INPUT_NOT_CONSUMED    (-8)

int lzo_init(void);

int lzo1x_decompress_safe(const lzo_byte* src, lzo_uint src_len,
                          lzo_byte* dst, lzo_uint* dst_len,
                          void* wrkmem /* NOT USED */);

#ifdef __cplusplus
}
#endif

#endif /* __MINILZO_H */

/*
 * This file is part of MPlayer CE.
 *
 * MPlayer CE is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * MPlayer CE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with MPlayer CE; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVUTIL_PPC_BSWAP_H
#define AVUTIL_PPC_BSWAP_H

#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"

#ifdef _XBOX

#define av_bswap16 av_bswap16
static av_always_inline av_const uint16_t av_bswap16(uint16_t x)
{
	return  __loadshortbytereverse(0,x);
}

#define av_bswap32 av_bswap32
static av_always_inline av_const uint32_t av_bswap32(uint32_t x)
{
	return __loadwordbytereverse(0,x);
}

#endif

#endif /* AVUTIL_PPC_BSWAP_H */
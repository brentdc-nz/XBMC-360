/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#ifndef H_CTIMEUTILS
#define H_CTIMEUTILS

#include <stdint.h>
#include <time.h>

enum TIME_FORMAT { TIME_FORMAT_GUESS     =  0,
                   TIME_FORMAT_SS        =  1,
                   TIME_FORMAT_MM        =  2,
                   TIME_FORMAT_MM_SS     =  3,
                   TIME_FORMAT_HH        =  4,
                   TIME_FORMAT_HH_SS     =  5, // not particularly useful
                   TIME_FORMAT_HH_MM     =  6,
                   TIME_FORMAT_HH_MM_SS  =  7,
                   TIME_FORMAT_XX        =  8, // AM/PM
                   TIME_FORMAT_HH_MM_XX  = 14,
                   TIME_FORMAT_H         = 16,
                   TIME_FORMAT_H_MM_SS   = 19 };

class CTimeUtils
{
public:
	static void UpdateFrameTime();      ///< update the frame time.  Not threadsafe
	static unsigned int GetFrameTime(); ///< returns the frame time in MS.  Not threadsafe
	static unsigned int GetTimeMS();

private:
	static unsigned int frameTime;
};

#endif //H_CTIMEUTILS


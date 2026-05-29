#pragma once

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

#include "Python.h"
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	typedef struct {
		PyObject_HEAD
		std::string director;
		std::string writingCredits;
		std::string genre;
		std::string tagLine;
		std::string plotOutline;
		std::string plot;
		std::string pictureURL;
		std::string title;
		std::string originalTitle;
		std::string votes;
		std::string cast;
		std::string file;
		std::string path;
		std::string IMDBNumber;
		int year;
		float rating;
		std::string premiered;
		std::string firstAired;
		int playCount;
		std::string lastPlayed;
	} InfoTagVideo;

	extern PyTypeObject InfoTagVideo_Type;

	void initInfoTagVideo_Type();
}

#ifdef __cplusplus
}
#endif

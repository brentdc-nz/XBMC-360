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

// =============================================================================
// Xbox 360 port stub for xbmc4xbox CPicture.
//
// Public API mirrors xbmc/pictures/Picture.h from xbmc4xbox so callsites in the
// port can be kept 1:1 with the source. The current implementation only wraps
// CFile::Cache / CCurlFile for remote fetches and does NOT decode or resize
// images (no JpegIO / DllImageLib equivalent ported yet). Replace the function
// bodies when the Xbox 360 image subsystem (D3D9 + libjpeg / etc.) is ported.
// =============================================================================

#include "utils\StdString.h"

class CPicture
{
public:
	CPicture(void);
	virtual ~CPicture(void);

	static bool CreateThumbnail(const CStdString& file, const CStdString& thumbFile, bool checkExistence = false);
	static bool CacheThumb(const CStdString& sourceUrl, const CStdString& destFile);
	static bool CacheFanart(const CStdString& sourceUrl, const CStdString& destFile);

	static bool CreateThumbnailFromMemory(const unsigned char* buffer, int bufSize, const CStdString& extension, const CStdString& thumbFile);
	static bool CreateThumbnailFromSurface(const unsigned char* buffer, int width, int height, int stride, const CStdString& thumbFile);

	static void CreateFolderThumb(const CStdString* strThumbs, const CStdString& folderThumbnail);

	// caches a skin image as a thumbnail image (stub - returns false)
	bool CacheSkinImage(const CStdString& srcFile, const CStdString& destFile);

private:
	static bool CacheImage(const CStdString& sourceUrl, const CStdString& destFile, int width, int height);
};

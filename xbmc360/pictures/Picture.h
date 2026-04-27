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

#include "utils\StdString.h"
#include <xtl.h>

#ifndef EXIF_MAX_COMMENT
#define EXIF_MAX_COMMENT 2000
#endif

typedef struct tag_ExifInfo {
	char  Version      [5];
	char  CameraMake   [32];
	char  CameraModel  [40];
	char  DateTime     [20];
	int   Height, Width;
	int   Orientation;
	int   IsColor;
	int   Process;
	int   FlashUsed;
	float FocalLength;
	float ExposureTime;
	float ApertureFNumber;
	float Distance;
	float CCDWidth;
	float ExposureBias;
	int   Whitebalance;
	int   MeteringMode;
	int   ExposureProgram;
	int   ISOequivalent;
	int   CompressionLevel;
	float FocalplaneXRes;
	float FocalplaneYRes;
	float FocalplaneUnits;
	float Xresolution;
	float Yresolution;
	float ResolutionUnit;
	float Brightness;
	char  Comments[EXIF_MAX_COMMENT];

	unsigned char * ThumbnailPointer;
	unsigned ThumbnailSize;

	bool  IsExif;
} EXIFINFO;

struct ImageInfo
{
	unsigned int width;
	unsigned int height;
	unsigned int originalwidth;
	unsigned int originalheight;
	EXIFINFO exifInfo;
	BYTE* texture;
	void* context;
	BYTE* alpha;
};

class CPicture
{
public:
	CPicture(void);
	virtual ~CPicture(void);

	LPDIRECT3DTEXTURE9 Load(const CStdString& strFilename, int width = 128, int height = 128);

	static bool CreateThumbnail(const CStdString& file, const CStdString& thumbFile, bool checkExistence = false);
	static bool CacheThumb(const CStdString& sourceUrl, const CStdString& destFile);
	static bool CacheFanart(const CStdString& sourceUrl, const CStdString& destFile);

	static bool CreateThumbnailFromMemory(const unsigned char* buffer, int bufSize, const CStdString& extension, const CStdString& thumbFile);
	static bool CreateThumbnailFromSurface(const unsigned char* buffer, int width, int height, int stride, const CStdString& thumbFile);

	static void CreateFolderThumb(const CStdString* strThumbs, const CStdString& folderThumbnail);

	bool CacheSkinImage(const CStdString& srcFile, const CStdString& destFile);

	ImageInfo GetInfo() const { return m_info; };
	unsigned int GetWidth() const { return m_info.width; };
	unsigned int GetHeight() const { return m_info.height; };
	unsigned int GetOriginalWidth() const { return m_info.originalwidth; };
	unsigned int GetOriginalHeight() const { return m_info.originalheight; };
	const EXIFINFO *GetExifInfo() const { return &m_info.exifInfo; };

private:
	static bool CacheImage(const CStdString& sourceUrl, const CStdString& destFile, int width, int height);

	ImageInfo m_info;
};

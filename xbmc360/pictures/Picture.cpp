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
// Xbox 360 port stub for xbmc4xbox CPicture. See Picture.h for rationale.
//
// Current behaviour:
//   - CreateThumbnail / CacheThumb / CacheFanart / CacheImage:
//       Ensure destination directory exists, then byte-copy source to dest via
//       CFile::Cache. No resize, no re-encode.
//   - CreateThumbnailFromMemory / CreateThumbnailFromSurface / CreateFolderThumb
//     / CacheSkinImage: not implemented, return false / no-op.
//
// When the real image subsystem is ported, only the bodies below need to
// change; callsites (ThumbLoader, FileItem, etc.) already use the correct API.
// =============================================================================

#include "Picture.h"
#include "filesystem\File.h"
#include "filesystem\Directory.h"
#include "utils\URIUtils.h"
#include "utils\Log.h"

using namespace XFILE;

CPicture::CPicture(void)
{
}

CPicture::~CPicture(void)
{
}

bool CPicture::CreateThumbnail(const CStdString& file, const CStdString& thumbFile, bool checkExistence /* = false */)
{
	// Don't create the thumb if it already exists
	if (checkExistence && CFile::Exists(thumbFile))
		return true;

	return CacheImage(file, thumbFile, 0, 0);
}

bool CPicture::CacheThumb(const CStdString& sourceUrl, const CStdString& destFile)
{
	return CacheImage(sourceUrl, destFile, 0, 0);
}

bool CPicture::CacheFanart(const CStdString& sourceUrl, const CStdString& destFile)
{
	return CacheImage(sourceUrl, destFile, 0, 0);
}

bool CPicture::CacheImage(const CStdString& sourceUrl, const CStdString& destFile, int /*width*/, int /*height*/)
{
	CLog::Log(LOGINFO, "CPicture: caching image from: %s to %s", sourceUrl.c_str(), destFile.c_str());

	// Ensure destination directory tree exists. CDirectory::Create only creates
	// the leaf, but thumb paths look like D:\Thumbnails\Video\<h>\<crc>.tbn
	// and any of those levels may be missing on first run. Walk the path and
	// create each missing level in turn.
	CStdString strDir;
	URIUtils::GetDirectory(destFile, strDir);
	if (!strDir.IsEmpty() && !CDirectory::Exists(strDir))
	{
		CStdString normalized(strDir);
		normalized.Replace("/", "\\");
		// Start after the drive letter / leading slashes.
		int pos = normalized.Find('\\');
		while (pos != -1)
		{
			CStdString partial = normalized.Left(pos);
			if (!partial.IsEmpty() && partial.GetLength() > 2 /* skip "D:" */)
			{
				if (!CDirectory::Exists(partial))
					CDirectory::Create(partial);
			}
			pos = normalized.Find('\\', pos + 1);
		}
		// Create the full (leaf) directory
		if (!CDirectory::Exists(normalized))
			CDirectory::Create(normalized);
	}

	// TODO: Port Xbox 360 image decode + resize subsystem. For now byte-copy
	// the source so we at least end up with a local cached copy (matches the
	// on-disk layout xbmc4xbox would produce, just without resizing).
	if (!CFile::Cache(sourceUrl, destFile))
	{
		CLog::Log(LOGERROR, "CPicture: failed to cache %s to %s", sourceUrl.c_str(), destFile.c_str());
		return false;
	}

	return true;
}

bool CPicture::CreateThumbnailFromMemory(const unsigned char* /*buffer*/, int /*bufSize*/, const CStdString& /*extension*/, const CStdString& thumbFile)
{
	// TODO: Port when image subsystem is ported (used by embedded music/video tag art)
	CLog::Log(LOGWARNING, "CPicture::CreateThumbnailFromMemory: not implemented (%s)", thumbFile.c_str());
	return false;
}

bool CPicture::CreateThumbnailFromSurface(const unsigned char* /*buffer*/, int /*width*/, int /*height*/, int /*stride*/, const CStdString& thumbFile)
{
	// TODO: Port when image subsystem is ported (used for video frame thumbs)
	CLog::Log(LOGWARNING, "CPicture::CreateThumbnailFromSurface: not implemented (%s)", thumbFile.c_str());
	return false;
}

void CPicture::CreateFolderThumb(const CStdString* /*thumbs*/, const CStdString& folderThumb)
{
	// TODO: Port when image subsystem is ported (4-tile folder mosaic)
	CLog::Log(LOGWARNING, "CPicture::CreateFolderThumb: not implemented (%s)", folderThumb.c_str());
}

bool CPicture::CacheSkinImage(const CStdString& /*srcFile*/, const CStdString& /*destFile*/)
{
	// TODO: Port when image subsystem is ported
	return false;
}

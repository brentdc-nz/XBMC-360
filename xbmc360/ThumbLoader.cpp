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
// Structure mirrored 1:1 from xbmc4xbox xbmc/ThumbLoader.cpp so DLNA/remote
// thumbs get the same caching behaviour (cached .tbn is reused across list
// redraws / playback transitions instead of being re-fetched every time).
//
// Things the Xbox 360 port does NOT do yet (stubbed in their matching source
// positions so this file can be diffed against xbmc4xbox cleanly):
//   - CVideoThumbLoader::SetWatchedOverlay hits the video database
//   - CFileItem::SetUserVideoThumb / SetUserMusicThumb / SetUserProgramThumb
//   - CFileItem::CacheLocalFanart / GetCachedFanart (returns empty)
//   - CPicture::CreateThumbnail is currently a byte-copy (no decode/resize).
// =============================================================================

#include "ThumbLoader.h"
#include "FileItem.h"
#include "pictures\Picture.h"
#include "filesystem\File.h"
#include "guilib\TextureManager.h"
#include "utils\Log.h"
#include "utils\URIUtils.h"

using namespace XFILE;

CThumbLoader::CThumbLoader(int nThreads) :
	CBackgroundInfoLoader(nThreads)
{
}

CThumbLoader::~CThumbLoader()
{
}

bool CThumbLoader::LoadRemoteThumb(CFileItem *pItem)
{
	// look for remote thumbs
	CStdString thumb(pItem->GetThumbnailImage());
	if (!g_TextureManager.CanLoad(thumb))
	{
		CStdString cachedThumb(pItem->GetCachedVideoThumb());
		if (CFile::Exists(cachedThumb))
			pItem->SetThumbnailImage(cachedThumb);
		else
		{
			CPicture pic;
			if (pic.CreateThumbnail(thumb, cachedThumb))
				pItem->SetThumbnailImage(cachedThumb);
			else
				pItem->SetThumbnailImage("");
		}
	}
	return pItem->HasThumbnail();
}

//=============================================================================

CVideoThumbLoader::CVideoThumbLoader() :
	CThumbLoader()
{
}

CVideoThumbLoader::~CVideoThumbLoader()
{
	StopThread();
}

void CVideoThumbLoader::OnLoaderStart()
{
}

void CVideoThumbLoader::OnLoaderFinish()
{
}

void CVideoThumbLoader::SetWatchedOverlay(CFileItem * /*item*/)
{
	// TODO: port CVideoDatabase::GetPlayCount and set ICON_OVERLAY_UNWATCHED/_WATCHED
}

bool CVideoThumbLoader::LoadItem(CFileItem* pItem)
{
	if (pItem->m_bIsShareOrDrive
	||  pItem->IsParentFolder())
		return false;

	SetWatchedOverlay(pItem);

	CFileItem item(*pItem);
	CStdString cachedThumb(item.GetCachedVideoThumb());

	if (!pItem->HasThumbnail())
	{
		item.SetUserVideoThumb();
		if (CFile::Exists(cachedThumb))
			pItem->SetThumbnailImage(cachedThumb);
		else
		{
			// create unique thumb for auto generated thumbs
			CStdString strPath, strFileName;
			URIUtils::GetDirectory(cachedThumb, strPath);
			strFileName = URIUtils::GetFileName(cachedThumb);
			cachedThumb = URIUtils::AddFileToFolder(strPath, "auto-" + strFileName);

			if (CFile::Exists(cachedThumb))
			{
				struct __stat64 stat;
				if (CFile::Stat(cachedThumb, &stat) == 0 && stat.st_size > 0)
				{
					pItem->SetProperty("HasAutoThumb", "1");
					pItem->SetProperty("AutoThumbImage", cachedThumb);
					pItem->SetThumbnailImage(cachedThumb);
				}
			}
		}
	}
	else if (!pItem->GetThumbnailImage().Left(10).Equals("special://"))
		LoadRemoteThumb(pItem);

	if (!pItem->HasProperty("fanart_image"))
	{
		if (pItem->CacheLocalFanart())
			pItem->SetProperty("fanart_image", pItem->GetCachedFanart());
	}

	return true;
}

//=============================================================================

CProgramThumbLoader::CProgramThumbLoader()
{
}

CProgramThumbLoader::~CProgramThumbLoader()
{
}

bool CProgramThumbLoader::LoadItem(CFileItem *pItem)
{
	if (pItem->m_bIsShareOrDrive) return true;
	if (!pItem->HasThumbnail())
		pItem->SetUserProgramThumb();
	else
		LoadRemoteThumb(pItem);
	return true;
}

//=============================================================================

CMusicThumbLoader::CMusicThumbLoader()
{
}

CMusicThumbLoader::~CMusicThumbLoader()
{
}

bool CMusicThumbLoader::LoadItem(CFileItem* pItem)
{
	if (pItem->m_bIsShareOrDrive) return true;
	if (!pItem->HasThumbnail())
		pItem->SetUserMusicThumb();
	else
		LoadRemoteThumb(pItem);
	return true;
}

//=============================================================================

CPictureThumbLoader::CPictureThumbLoader()
{
}

CPictureThumbLoader::~CPictureThumbLoader()
{
}

bool CPictureThumbLoader::LoadItem(CFileItem* pItem)
{
	if (pItem->m_bIsShareOrDrive) return true;
	if (pItem->HasThumbnail())
		LoadRemoteThumb(pItem);
	return true;
}

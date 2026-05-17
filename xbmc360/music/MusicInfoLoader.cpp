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

#include "music\MusicInfoLoader.h"
#include "music\tags\MusicInfoTagLoaderFactory.h"
#include "utils\URIUtils.h"
#include "music\tags\MusicInfoTag.h"
#include "filesystem\File.h"
#include "FileItem.h"
#include "utils\Log.h"

using namespace std;
using namespace XFILE;
using namespace MUSIC_INFO;

// HACK until we make this threadable - specify 1 thread only for now
CMusicInfoLoader::CMusicInfoLoader() : CBackgroundInfoLoader(1)
{
	m_mapFileItems = new CFileItemList;
}

CMusicInfoLoader::~CMusicInfoLoader()
{
	StopThread();
	delete m_mapFileItems;
}

void CMusicInfoLoader::OnLoaderStart()
{
	// Load previously cached items from HD
	if (!m_strCacheFileName.IsEmpty())
		LoadCache(m_strCacheFileName, *m_mapFileItems);
	else
	{
		m_mapFileItems->SetPath(m_pVecItems->GetPath());
		m_mapFileItems->Load();
		m_mapFileItems->SetFastLookup(true);
	}

	m_strPrevPath.Empty();

	m_databaseHits = m_tagReads = 0;

	if (m_pProgressCallback)
		m_pProgressCallback->SetProgressMax(m_pVecItems->GetFileCount());
}

bool CMusicInfoLoader::LoadAdditionalTagInfo(CFileItem* pItem)
{
	if (!pItem || pItem->m_bIsFolder || pItem->IsPlayList() || pItem->IsNFO() || pItem->IsInternetStream())
		return false;

	if (pItem->GetProperty("hasfullmusictag") == "true")
		return false; // already have the information

	CStdString path(pItem->GetPath());

	CLog::Log(LOGDEBUG, "Loading additional tag info for file %s", path.c_str());

	// we load up the actual tag for this file
	CMusicInfoTag tag;
	IMusicInfoTagLoader* pLoader = CMusicInfoTagLoaderFactory::CreateLoader(path);
	if (NULL != pLoader)
	{
		pLoader->Load(path, tag);
		// then we set the fields from the file tags to the item
		pItem->SetProperty("lyrics", tag.GetLyrics());
		pItem->SetProperty("hasfullmusictag", "true");
		delete pLoader;
		return true;
	}
	return false;
}

bool CMusicInfoLoader::LoadItem(CFileItem* pItem)
{
	if (m_pProgressCallback && !pItem->m_bIsFolder)
		m_pProgressCallback->SetProgressAdvance();

	if (pItem->m_bIsFolder || pItem->IsPlayList() || pItem->IsNFO() || pItem->IsInternetStream())
		return false;

	if (pItem->HasMusicInfoTag() && pItem->GetMusicInfoTag()->Loaded())
		return true;

	// first check the cached item
	CFileItemPtr mapItem = (*m_mapFileItems)[pItem->GetPath()];
	if (mapItem && mapItem->m_dateTime==pItem->m_dateTime && mapItem->HasMusicInfoTag() && mapItem->GetMusicInfoTag()->Loaded())
	{ // Query map if we previously cached the file on HD
		*pItem->GetMusicInfoTag() = *mapItem->GetMusicInfoTag();
		pItem->SetThumbnailImage(mapItem->GetThumbnailImage());
		return true;
	}

	// No music database - load tag from file directly
	// get correct tag parser
	IMusicInfoTagLoader* pLoader = CMusicInfoTagLoaderFactory::CreateLoader(pItem->GetPath());
	if (NULL != pLoader)
	{
		// get tag
		pLoader->Load(pItem->GetPath(), *pItem->GetMusicInfoTag());
		delete pLoader;
	}
	m_tagReads++;

	return true;
}

void CMusicInfoLoader::OnLoaderFinish()
{
	// cleanup cache loaded from HD
	m_mapFileItems->Clear();

	// Save loaded items to HD
	if (!m_strCacheFileName.IsEmpty())
		SaveCache(m_strCacheFileName, *m_pVecItems);
	else if (!m_bStop && (m_databaseHits > 1 || m_tagReads > 0))
		m_pVecItems->Save();
}

void CMusicInfoLoader::UseCacheOnHD(const CStdString& strFileName)
{
	m_strCacheFileName = strFileName;
}

void CMusicInfoLoader::LoadCache(const CStdString& strFileName, CFileItemList& items)
{
	// TODO: CFileItem doesn't implement IArchivable yet - can't serialize with CArchive
}

void CMusicInfoLoader::SaveCache(const CStdString& strFileName, CFileItemList& items)
{
	// TODO: CFileItem doesn't implement IArchivable yet - can't serialize with CArchive
}

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

#include "music\tags\MusicInfoTagLoaderFactory.h"
#include "music\tags\MusicInfoTagLoaderMP3.h"
#include "utils\URIUtils.h"
#include "FileItem.h"

using namespace MUSIC_INFO;

CMusicInfoTagLoaderFactory::CMusicInfoTagLoaderFactory()
{}

CMusicInfoTagLoaderFactory::~CMusicInfoTagLoaderFactory()
{}

IMusicInfoTagLoader* CMusicInfoTagLoaderFactory::CreateLoader(const CStdString& strFileName)
{
	// dont try to read the tags for streams & shoutcast
	CFileItem item(strFileName, false);
	if (item.IsInternetStream())
		return NULL;

	CStdString strExtension;
	URIUtils::GetExtension(strFileName, strExtension);
	strExtension.ToLower();
	strExtension.TrimLeft('.');

	if (strExtension.IsEmpty())
		return NULL;

	if (strExtension == "mp3")
	{
		CMusicInfoTagLoaderMP3 *pTagLoader = new CMusicInfoTagLoaderMP3();
		return (IMusicInfoTagLoader*)pTagLoader;
	}

	return NULL;
}

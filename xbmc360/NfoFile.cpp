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
// NfoFile.cpp: implementation of the CNfoFile class.
//
//////////////////////////////////////////////////////////////////////

#include "NfoFile.h"
#include "video\VideoInfoTag.h"
#include "filesystem\File.h"
#include "filesystem\Directory.h"
#include "utils\URIUtils.h"
#include "FileItem.h"
#include <vector>
#include "utils\log.h"

using namespace XFILE;
using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNfoFile::CNfoFile()
{
	m_doc = NULL;
	m_headofdoc = NULL;
}

CNfoFile::~CNfoFile()
{
	Close();
}

CNfoFile::NFOResult CNfoFile::Create(const CStdString& strPath, const CStdString& strContent, int episode)
{
	if (FAILED(Load(strPath)))
		return NO_NFO;

	// first check if it's an XML file with the info we need
	CVideoInfoTag details;
	bool bNfo = GetDetails(details);

	if (episode > -1 && bNfo && strContent.Equals("tvshows"))
	{
		int infos=0;
		while (m_headofdoc && details.m_iEpisode != episode)
		{
			m_headofdoc = strstr(m_headofdoc+1,"<episodedetails>");
			bNfo  = GetDetails(details);
			infos++;
		}
		if (details.m_iEpisode != episode)
		{
			bNfo = false;
			details.Reset();
			m_headofdoc = m_doc;
			if (infos == 1) // still allow differing nfo/file numbers for single ep nfo's
				bNfo = GetDetails(details);
		}
	}

	if (bNfo)
		return FULL_NFO;

	return NO_NFO;
}

int CNfoFile::Load(const CStdString& strFile)
{
	Close();
	XFILE::CFile file;
	if (file.Open(strFile))
	{
		m_size = (int)file.GetLength();
		try
		{
			m_doc = new char[m_size+1];
			m_headofdoc = m_doc;
		}
		catch (...)
		{
			CLog::Log(LOGERROR, "%s: Exception while creating file buffer",__FUNCTION__);
			return 1;
		}
		if (!m_doc)
		{
			file.Close();
			return 1;
		}
		file.Read(m_doc, m_size);
		m_doc[m_size] = 0;
		file.Close();
		return 0;
	}
	return 1;
}

void CNfoFile::Close()
{
	if (m_doc != NULL)
	{
		delete m_doc;
		m_doc = 0;
	}

	m_strImDbNr = "";
	m_size = 0;
}

CStdString CNfoFile::GetnfoFile(CFileItem *item, bool bGrabAny)
{
	CStdString nfoFile;
	// Find a matching .nfo file
	if (!item->m_bIsFolder)
	{
		// file
		CStdString strExtension;
		URIUtils::GetExtension(item->GetPath(), strExtension);

		if (URIUtils::IsInRAR(item->GetPath())) // we have a rarred item - we want to check outside the rars
		{
			CFileItem item2(*item);
			CURL url(item->GetPath());
			CStdString strPath;
			URIUtils::GetDirectory(url.GetHostName(), strPath);
			item2.SetPath(URIUtils::AddFileToFolder(strPath, URIUtils::GetFileName(item->GetPath())));
			return GetnfoFile(&item2, bGrabAny);
		}

		// grab the folder path
		CStdString strPath;
		URIUtils::GetDirectory(item->GetPath(), strPath);

		if (bGrabAny)
		{ // looking up by folder name - movie.nfo and mymovies.xml take priority
			nfoFile = URIUtils::AddFileToFolder(strPath, "movie.nfo");
			if (CFile::Exists(nfoFile))
				return nfoFile;
			nfoFile = URIUtils::AddFileToFolder(strPath, "mymovies.xml");
			if (CFile::Exists(nfoFile))
				return nfoFile;
		}

		// try looking for .nfo file for a stacked item
		if (item->IsStack())
		{
			// TODO: Stack file support not ported
			// In xbmc4xbox this uses CStackDirectory to get first stacked file
			// and stacked title path, then recursively calls GetnfoFile
		}
		else
		{
			// already an .nfo file?
			if ( strcmpi(strExtension.c_str(), ".nfo") == 0 )
				nfoFile = item->GetPath();
			// no, create .nfo file
			else
				nfoFile = URIUtils::ReplaceExtension(item->GetPath(), ".nfo");
		}

		// test file existence
		if (!nfoFile.IsEmpty() && !CFile::Exists(nfoFile))
			nfoFile.Empty();

		if (nfoFile.IsEmpty()) // final attempt - strip off any cd1 folders
		{
			URIUtils::RemoveSlashAtEnd(strPath); // need no slash for the check that follows
			CFileItem item2;
			if (strPath.Mid(strPath.size()-3).Equals("cd1"))
			{
				strPath = strPath.Mid(0,strPath.size()-3);
				item2.SetPath(URIUtils::AddFileToFolder(strPath, URIUtils::GetFileName(item->GetPath())));
				return GetnfoFile(&item2, bGrabAny);
			}
		}
	}
	// folders can take any nfo file if there's a unique one
	if (item->m_bIsFolder || (bGrabAny && nfoFile.IsEmpty()))
	{
		// see if there is a unique nfo file in this folder, and if so, use that
		CFileItemList items;
		CDirectory dir;
		CStdString strPath = item->GetPath();
		if (!item->m_bIsFolder)
			URIUtils::GetDirectory(item->GetPath(), strPath);
		if (dir.GetDirectory(strPath, items, ".nfo") && items.Size())
		{
			int numNFO = -1;
			for (int i = 0; i < items.Size(); i++)
			{
				if (URIUtils::GetExtension(items[i]->GetPath()).Equals(".nfo"))
				{
					if (numNFO == -1)
						numNFO = i;
					else
					{
						numNFO = -1;
						break;
					}
				}
			}
			if (numNFO > -1)
				return items[numNFO]->GetPath();
		}
	}

	return nfoFile;
}

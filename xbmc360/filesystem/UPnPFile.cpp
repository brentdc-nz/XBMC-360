/*
 *      Copyright (C) 2011 Team XBMC
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

#include "UPnPFile.h"
#include "UPnPDirectory.h"
#include "FileFactory.h"
#include "..\FileItem.h"
#include "..\utils\Log.h"
#include "..\URL.h"

using namespace XFILE;

CUPnPFile::CUPnPFile()
{
}

CUPnPFile::~CUPnPFile()
{
}

bool CUPnPFile::Open(const CURL& url)
{
    CFileItem item;
    if (CUPnPDirectory::GetResource(url, item))
    {
        CLog::Log(LOGDEBUG, "CUPnPFile::Open - redirect to %s", item.m_strPath.c_str());

        IFile *pNewImp = CFileFactory::CreateLoader(item.m_strPath);
        CURL *pNewUrl = new CURL(item.m_strPath);

        if (pNewImp)
            throw new CRedirectException(pNewImp, pNewUrl);

        delete pNewUrl;
    }

    CLog::Log(LOGERROR, "CUPnPFile::Open - failed to get resource for %s", url.Get().c_str());
    return false;
}

int CUPnPFile::Stat(const CURL& url, struct __stat64* buffer)
{
    CFileItem item;
    if (CUPnPDirectory::GetResource(url, item))
    {
        IFile *pNewImp = CFileFactory::CreateLoader(item.m_strPath);
        CURL *pNewUrl = new CURL(item.m_strPath);

        if (pNewImp)
            throw new CRedirectException(pNewImp, pNewUrl);

        delete pNewUrl;
    }

    return -1;
}

bool CUPnPFile::Exists(const CURL& url)
{
    CFileItem item;
    if (CUPnPDirectory::GetResource(url, item))
    {
        IFile *pNewImp = CFileFactory::CreateLoader(item.m_strPath);
        CURL *pNewUrl = new CURL(item.m_strPath);

        if (pNewImp)
            throw new CRedirectException(pNewImp, pNewUrl);

        delete pNewUrl;
    }

    return false;
}
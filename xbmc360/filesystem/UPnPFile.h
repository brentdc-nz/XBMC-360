#pragma once
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

#include "FileBase.h"

namespace XFILE
{

class CUPnPFile : public CFileBase
{
public:
    CUPnPFile();
    virtual ~CUPnPFile();

    virtual bool Open(const CURL& url, bool bBinary = true);
    virtual int Stat(const CURL& url, struct __stat64* buffer);

    // These should never be called — Open throws CRedirectException
    virtual unsigned int Read(void* lpBuf, int64_t uiBufSize) { return 0; }
    virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET) { return -1; }
    virtual void Close() {}
    virtual int64_t GetPosition() { return -1; }
    virtual int64_t GetLength() { return -1; }
};

}

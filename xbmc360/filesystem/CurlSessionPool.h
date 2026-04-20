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
 *  Ported from xbmc4xbox DllLibCurlGlobal — session pool for libcurl handles.
 *  Uses statically linked libcurl instead of DLL wrapper.
 */

#include "..\utils\SingleLock.h"
#include "..\utils\StdString.h"
#include <vector>

// Forward declare curl types to avoid pulling in curl headers here
typedef void CURL_HANDLE;
typedef void CURLM;

class CCurlSessionPool
{
public:
    CCurlSessionPool();
    ~CCurlSessionPool();

    // Acquire a curl session for the given protocol+hostname.
    // Reuses an existing idle session to the same host if available.
    void easy_aquire(const char* protocol, const char* hostname,
                     CURL_HANDLE** easy_handle, CURLM** multi_handle);

    // Release a session back to the pool for reuse.
    void easy_release(CURL_HANDLE** easy_handle, CURLM** multi_handle);

    // Clean up idle sessions that have exceeded their timeout.
    // Call periodically from the application main loop.
    void CheckIdle();

private:
    typedef struct SSession
    {
        DWORD         m_idletimestamp;
        CStdString    m_protocol;
        CStdString    m_hostname;
        bool          m_busy;
        CURL_HANDLE*  m_easy;
        CURLM*        m_multi;
    } SSession;

    typedef std::vector<SSession> VEC_CURLSESSIONS;

    VEC_CURLSESSIONS m_sessions;
    CCriticalSection m_critSection;
    bool             m_initialized;

    void EnsureInit();
};

extern CCurlSessionPool g_curlSessionPool;

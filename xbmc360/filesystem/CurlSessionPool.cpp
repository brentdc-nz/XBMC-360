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

// Include Xbox headers first so _WINSOCKAPI_ is defined, preventing
// curl.h from trying to include winsock2.h (Xbox 360 uses winsockx.h)
#include <xtl.h>
#include <winsockx.h>

#define CURL CURL_HANDLE
#include <curl/curl.h>
#undef CURL

#include "CurlSessionPool.h"
#include "..\utils\Log.h"

CCurlSessionPool g_curlSessionPool;

CCurlSessionPool::CCurlSessionPool()
{
    m_initialized = false;
}

CCurlSessionPool::~CCurlSessionPool()
{
    CSingleLock lock(m_critSection);

    VEC_CURLSESSIONS::iterator it;
    for (it = m_sessions.begin(); it != m_sessions.end(); it++)
    {
        // Clean up multi *before* easy, because multi cleanup accesses easy's structure
        if (it->m_multi)
            curl_multi_cleanup((CURLM*)it->m_multi);
        if (it->m_easy)
            curl_easy_cleanup(it->m_easy);
    }
    m_sessions.clear();

    if (m_initialized)
    {
        curl_global_cleanup();
        m_initialized = false;
    }
}

void CCurlSessionPool::EnsureInit()
{
    if (!m_initialized)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        m_initialized = true;
    }
}

void CCurlSessionPool::easy_aquire(const char* protocol, const char* hostname,
                                   CURL_HANDLE** easy_handle, CURLM** multi_handle)
{
    CSingleLock lock(m_critSection);
    EnsureInit();

    VEC_CURLSESSIONS::iterator it;
    for (it = m_sessions.begin(); it != m_sessions.end(); it++)
    {
        if (!it->m_busy)
        {
            // Allow reuse if requester is trying to connect to same host
            // curl will take care of any differences in username/password
            if (it->m_protocol.compare(protocol) == 0 && it->m_hostname.compare(hostname) == 0)
            {
                it->m_busy = true;

                if (easy_handle)
                {
                    if (!it->m_easy)
                        it->m_easy = curl_easy_init();
                    *easy_handle = it->m_easy;
                }

                if (multi_handle)
                {
                    if (!it->m_multi)
                        it->m_multi = curl_multi_init();
                    *multi_handle = it->m_multi;
                }

                return;
            }
        }
    }

    // No existing session found — create a new one
    SSession session = {};
    session.m_busy = true;
    session.m_protocol = protocol;
    session.m_hostname = hostname;

    if (easy_handle)
    {
        session.m_easy = curl_easy_init();
        *easy_handle = session.m_easy;
    }

    if (multi_handle)
    {
        session.m_multi = curl_multi_init();
        *multi_handle = session.m_multi;
    }

    m_sessions.push_back(session);

    CLog::Log(LOGINFO, "%s - Created session to %s://%s", __FUNCTION__, protocol, hostname);
}

void CCurlSessionPool::easy_release(CURL_HANDLE** easy_handle, CURLM** multi_handle)
{
    CSingleLock lock(m_critSection);

    CURL_HANDLE* easy = NULL;
    CURLM*       multi = NULL;

    if (easy_handle)
    {
        easy = *easy_handle;
        *easy_handle = NULL;
    }

    if (multi_handle)
    {
        multi = *multi_handle;
        *multi_handle = NULL;
    }

    VEC_CURLSESSIONS::iterator it;
    for (it = m_sessions.begin(); it != m_sessions.end(); it++)
    {
        if (it->m_easy == easy && (multi == NULL || it->m_multi == multi))
        {
            // Reset session so next caller doesn't reuse options, only connections
            curl_easy_reset(easy);
            it->m_busy = false;
            it->m_idletimestamp = GetTickCount();
            return;
        }
    }
}

void CCurlSessionPool::CheckIdle()
{
    CSingleLock lock(m_critSection);

    // 10 seconds idle time before closing handle — keep low on Xbox 360 to save memory
    const DWORD idletime = 10000;

    VEC_CURLSESSIONS::iterator it = m_sessions.begin();
    while (it != m_sessions.end())
    {
        if (!it->m_busy && it->m_idletimestamp + idletime < GetTickCount())
        {
            CLog::Log(LOGINFO, "%s - Closing session to %s://%s (easy=%p, multi=%p)",
                      __FUNCTION__, it->m_protocol.c_str(), it->m_hostname.c_str(),
                      (void*)it->m_easy, (void*)it->m_multi);

            // Clean up multi *before* easy
            if (it->m_multi)
                curl_multi_cleanup((CURLM*)it->m_multi);
            if (it->m_easy)
                curl_easy_cleanup(it->m_easy);

            it = m_sessions.erase(it);
            continue;
        }
        it++;
    }
}

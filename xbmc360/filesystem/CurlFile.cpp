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
 *  Ported from xbmc4xbox CCurlFile — simplified for Xbox 360
 *  Uses statically linked libcurl instead of DllLibCurl wrapper
 */

// Include Xbox headers first so _WINSOCKAPI_ is defined, preventing
// curl.h from trying to include winsock2.h (Xbox 360 uses winsockx.h)
#include <xtl.h>
#include <winsockx.h>

// Include curl — rename CURL to CURL_HANDLE to avoid clash with our CURL URL class
#define CURL CURL_HANDLE
#include <curl/curl.h>
#undef CURL

#include "CurlFile.h"
#include "CurlSessionPool.h"
#include "..\utils\Log.h"
#include "..\utils\SystemInfo.h"
#include "..\URL.h"
#include "..\Application.h"
#include "..\AdvancedSettings.h"

using namespace XFILE;

#define XMIN(a,b) ((a)<(b)?(a):(b))
#define FITS_INT(a) (((a) <= INT_MAX) && ((a) >= INT_MIN))

//------------------------------------------------------------------------------
// C callbacks for curl
//------------------------------------------------------------------------------

extern "C" size_t xbmc_write_cb(char *buffer, size_t size, size_t nitems, void *userp)
{
    if (!userp) return 0;
    CCurlFile::CReadState *state = (CCurlFile::CReadState *)userp;
    return state->WriteCallback(buffer, size, nitems);
}

extern "C" size_t xbmc_header_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    CCurlFile::CReadState *state = (CCurlFile::CReadState *)stream;
    return state->HeaderCallback(ptr, size, nmemb);
}

#if defined(_XBOX)
// Xbox 360 XNet requires socket option 0x5801 to allow TCP traffic to
// non-Xbox peers. 0x5802 must NOT be set on TCP sockets — it corrupts
// the TCP handshake (XNet reports "bad ACK seq in SYN_SENT" and fails
// with WSAEHOSTUNREACH 10065).
extern "C" int xbmc_curl_sockopt_cb(void* /*clientp*/, curl_socket_t curlfd, curlsocktype /*purpose*/)
{
    BOOL option = TRUE;
    int r = setsockopt((SOCKET)curlfd, SOL_SOCKET, 0x5801, (const char*)&option, sizeof(option));
    return CURL_SOCKOPT_OK;
}
#endif

/* fix for silly behavior of realloc */
static inline void* realloc_simple(void *ptr, size_t size)
{
    void *ptr2 = realloc(ptr, size);
    if (ptr && !ptr2 && size > 0)
    {
        free(ptr);
        return NULL;
    }
    return ptr2;
}

//------------------------------------------------------------------------------
// CReadState implementation
//------------------------------------------------------------------------------

CCurlFile::CReadState::CReadState()
{
    m_easyHandle = NULL;
    m_multiHandle = NULL;
    m_overflowBuffer = NULL;
    m_overflowSize = 0;
    m_filePos = 0;
    m_fileSize = 0;
    m_bufferSize = 0;
    m_cancelled = false;
    m_bFirstLoop = true;
    m_sendRange = true;
    m_isPaused = false;
    m_headerdone = false;
    m_stillRunning = 0;
    m_readBuffer = NULL;
}

CCurlFile::CReadState::~CReadState()
{
    Disconnect();
}

size_t CCurlFile::CReadState::HeaderCallback(void *ptr, size_t size, size_t nmemb)
{
    // Clear any previous headers on new response (e.g. after redirect)
    if (m_headerdone)
    {
        m_httpheader.Clear();
        m_headerdone = false;
    }

    // libcurl doc says this data is not always null-terminated
    int iSize = size * nmemb;
    char* strData = (char*)malloc(iSize + 1);
    if (!strData)
        return 0;
    memcpy(strData, ptr, iSize);
    strData[iSize] = 0;

    if (strcmp(strData, "\r\n") == 0)
        m_headerdone = true;

    m_httpheader.Parse(strData);

    free(strData);
    return iSize;
}

size_t CCurlFile::CReadState::WriteCallback(char *buffer, size_t size, size_t nitems)
{
    unsigned int amount = size * nitems;

    if (m_overflowSize)
    {
        unsigned int maxWriteable = XMIN((unsigned int)m_buffer.getMaxWriteSize(), m_overflowSize);
        if (maxWriteable)
        {
            if (!m_buffer.WriteData(m_overflowBuffer, maxWriteable))
                CLog::Log(LOGERROR, "CCurlFile::WriteCallback - Unable to write to buffer");
            if (m_overflowSize > maxWriteable)
                memmove(m_overflowBuffer, m_overflowBuffer + maxWriteable, m_overflowSize - maxWriteable);
            m_overflowSize -= maxWriteable;
        }
    }

    unsigned int maxWriteable = XMIN((unsigned int)m_buffer.getMaxWriteSize(), amount);
    if (maxWriteable)
    {
        if (!m_buffer.WriteData(buffer, maxWriteable))
        {
            CLog::Log(LOGERROR, "CCurlFile::WriteCallback - Unable to write to buffer with %i bytes", maxWriteable);
        }
        else
        {
            amount -= maxWriteable;
            buffer += maxWriteable;
        }
    }

    if (amount)
    {
        m_overflowBuffer = (char*)realloc_simple(m_overflowBuffer, amount + m_overflowSize);
        if (m_overflowBuffer == NULL)
        {
            CLog::Log(LOGWARNING, "CCurlFile::WriteCallback - Failed to grow overflow buffer");
            return 0;
        }
        memcpy(m_overflowBuffer + m_overflowSize, buffer, amount);
        m_overflowSize += amount;
    }

    return size * nitems;
}

bool CCurlFile::CReadState::Seek(int64_t pos)
{
    if (pos == m_filePos)
        return true;

    if (FITS_INT(pos - m_filePos) && m_buffer.SkipBytes((int)(pos - m_filePos)))
    {
        m_filePos = pos;
        return true;
    }

    if (pos > m_filePos && pos < m_filePos + m_bufferSize)
    {
        int len = m_buffer.getMaxReadSize();
        m_filePos += len;
        m_buffer.SkipBytes(len);
        if (!FillBuffer(m_bufferSize))
        {
            if (!m_buffer.SkipBytes(-len))
                CLog::Log(LOGERROR, "%s - Failed to restore position after failed fill", __FUNCTION__);
            else
                m_filePos -= len;
            return false;
        }

        if (!FITS_INT(pos - m_filePos) || !m_buffer.SkipBytes((int)(pos - m_filePos)))
        {
            CLog::Log(LOGERROR, "%s - Failed to skip to position after fill", __FUNCTION__);
            if (!m_buffer.SkipBytes(-len))
                CLog::Log(LOGERROR, "%s - Failed to restore position after failed seek", __FUNCTION__);
            else
                m_filePos -= len;
            return false;
        }
        m_filePos = pos;
        return true;
    }

    return false;
}

void CCurlFile::CReadState::SetResume()
{
    if (m_sendRange && m_filePos == 0)
        curl_easy_setopt(m_easyHandle, CURLOPT_RANGE, "0-");
    else
    {
        curl_easy_setopt(m_easyHandle, CURLOPT_RANGE, NULL);
        m_sendRange = false;
    }

    curl_easy_setopt(m_easyHandle, CURLOPT_RESUME_FROM_LARGE, (curl_off_t)m_filePos);
}

long CCurlFile::CReadState::Connect(unsigned int size)
{
    SetResume();
    curl_multi_add_handle((CURLM*)m_multiHandle, m_easyHandle);

    m_bufferSize = size;
    m_buffer.Destroy();
    m_buffer.Create(size * 3);
    m_headerdone = false;

    // read some data to try and obtain the length
    m_stillRunning = 1;
    if (!FillBuffer(1))
    {
        CLog::Log(LOGERROR, "CCurlFile::CReadState::Connect - didn't get any data from stream.");
        return -1;
    }

    double length;
    if (CURLE_OK == curl_easy_getinfo(m_easyHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length))
    {
        if (length < 0)
            length = 0.0;
        m_fileSize = m_filePos + (int64_t)length;
    }

    long response;
    if (CURLE_OK == curl_easy_getinfo(m_easyHandle, CURLINFO_RESPONSE_CODE, &response))
        return response;

    return -1;
}

void CCurlFile::CReadState::Disconnect()
{
    if (m_multiHandle && m_easyHandle)
        curl_multi_remove_handle((CURLM*)m_multiHandle, m_easyHandle);

    m_buffer.Clear();
    free(m_overflowBuffer);
    m_overflowBuffer = NULL;
    m_overflowSize = 0;
    m_filePos = 0;
    m_fileSize = 0;
    m_bufferSize = 0;
    m_readBuffer = NULL;
}

unsigned int CCurlFile::CReadState::Read(void* lpBuf, int64_t uiBufSize)
{
    /* only request 1 byte, for truncated reads (only if not eof) */
    if ((m_fileSize == 0 || m_filePos < m_fileSize) && !FillBuffer(1))
        return 0;

    /* ensure only available data is considered */
    unsigned int want = (unsigned int)XMIN(m_buffer.getMaxReadSize(), uiBufSize);

    /* xfer data to caller */
    if (m_buffer.ReadData((char *)lpBuf, want))
    {
        m_filePos += want;
        return want;
    }

    /* check if we finished prematurely */
    if (!m_stillRunning && (m_fileSize == 0 || m_filePos != m_fileSize))
    {
        CLog::Log(LOGWARNING, "%s - Transfer ended before entire file was retrieved pos %I64d, size %I64d",
                  __FUNCTION__, m_filePos, m_fileSize);
        return 0;
    }

    return 0;
}

bool CCurlFile::CReadState::ReadString(char *szLine, int iLineLength)
{
    unsigned int want = (unsigned int)iLineLength;

    if ((m_fileSize == 0 || m_filePos < m_fileSize) && !FillBuffer(want))
        return false;

    // ensure only available data is considered
    want = XMIN((unsigned int)m_buffer.getMaxReadSize(), want);

    /* check if we finished prematurely */
    if (!m_stillRunning && (m_fileSize == 0 || m_filePos != m_fileSize) && !want)
    {
        if (m_fileSize != 0)
            CLog::Log(LOGWARNING, "%s - Transfer ended before entire file was retrieved pos %I64d, size %I64d",
                      __FUNCTION__, m_filePos, m_fileSize);

        return false;
    }

    char* pLine = szLine;
    do
    {
        if (!m_buffer.ReadData(pLine, 1))
            break;

        pLine++;
    } while (((pLine - 1)[0] != '\n') && ((unsigned int)(pLine - szLine) < want));
    pLine[0] = 0;
    m_filePos += (pLine - szLine);
    return (bool)((pLine - szLine) > 0);
}

bool CCurlFile::CReadState::FillBuffer(unsigned int want)
{
    int retry = 0;
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;

    while ((unsigned int)m_buffer.getMaxReadSize() < want && m_buffer.getMaxWriteSize() > 0)
    {
        if (m_cancelled)
            return false;

        /* if there is data in overflow buffer, try to use that first */
        if (m_overflowSize)
        {
            unsigned amount = XMIN((unsigned int)m_buffer.getMaxWriteSize(), m_overflowSize);
            m_buffer.WriteData(m_overflowBuffer, amount);

            if (amount < m_overflowSize)
                memcpy(m_overflowBuffer, m_overflowBuffer + amount, m_overflowSize - amount);

            m_overflowSize -= amount;
            m_overflowBuffer = (char*)realloc_simple(m_overflowBuffer, m_overflowSize);
            continue;
        }

        CURLMcode result = curl_multi_perform((CURLM*)m_multiHandle, &m_stillRunning);
        if (!m_stillRunning)
        {
            if (result == CURLM_OK)
            {
                if (m_buffer.getMaxReadSize())
                    return true;

                // Verify that we are actually okey
                int msgs;
                CURLcode CURLresult = CURLE_OK;
                CURLMsg* msg;
                while ((msg = curl_multi_info_read((CURLM*)m_multiHandle, &msgs)))
                {
                    if (msg->msg == CURLMSG_DONE)
                    {
                        if (msg->data.result == CURLE_OK)
                            return true;

                        CLog::Log(LOGERROR, "CCurlFile::FillBuffer - Failed: %s(%d)",
                                  curl_easy_strerror(msg->data.result), msg->data.result);

                        // We need to check the result here as we don't want to retry on every error
                        if ((msg->data.result == CURLE_OPERATION_TIMEDOUT ||
                             msg->data.result == CURLE_PARTIAL_FILE       ||
                             msg->data.result == CURLE_COULDNT_CONNECT    ||
                             msg->data.result == CURLE_RECV_ERROR)        &&
                             !m_bFirstLoop)
                        {
                            CURLresult = msg->data.result;
                        }
                        else if ((msg->data.result == CURLE_HTTP_RANGE_ERROR     ||
                                  msg->data.result == CURLE_HTTP_RETURNED_ERROR) &&
                                  m_bFirstLoop && m_filePos == 0 && m_sendRange)
                        {
                        	// If server returns a range or http error, retry with range disabled
                            CURLresult = msg->data.result;
                            m_sendRange = false;
                        }
                        else
                            return false;
                    }
                }

                // Don't retry when we didn't "see" any error
                if (CURLresult == CURLE_OK)
                    return false;

                // Close handle for reconnect
                if (m_multiHandle && m_easyHandle)
                    curl_multi_remove_handle((CURLM*)m_multiHandle, m_easyHandle);

                m_buffer.Clear();
                free(m_overflowBuffer);
                m_overflowBuffer = NULL;
                m_overflowSize = 0;

                if (++retry > g_advancedSettings.m_curlretries)
                {
                    CLog::Log(LOGERROR, "CCurlFile::FillBuffer - Reconnect failed!");
                    m_filePos = 0;
                    m_fileSize = 0;
                    m_bufferSize = 0;
                    return false;
                }

                CLog::Log(LOGNOTICE, "CCurlFile::FillBuffer - Reconnect, (re)try %i", retry);

                SetResume();
                curl_multi_add_handle((CURLM*)m_multiHandle, m_easyHandle);
                continue;
            }
            return false;
        }

        if (m_bFirstLoop && m_buffer.getMaxReadSize() > 0)
            m_bFirstLoop = false;

        switch (result)
        {
        case CURLM_OK:
        {
            int maxfd = -1;
            FD_ZERO(&fdread);
            FD_ZERO(&fdwrite);
            FD_ZERO(&fdexcep);

            curl_multi_fdset((CURLM*)m_multiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);

            long timeout = 0;
            if (CURLM_OK != curl_multi_timeout((CURLM*)m_multiHandle, &timeout) || timeout == -1)
                timeout = 200;

            struct timeval t = { timeout / 1000, (timeout % 1000) * 1000 };

            if (SOCKET_ERROR == select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &t))
            {
                CLog::Log(LOGERROR, "CCurlFile::FillBuffer - Failed with socket error");
                return false;
            }
        }
        break;
        case CURLM_CALL_MULTI_PERFORM:
            continue;
        default:
            CLog::Log(LOGERROR, "CCurlFile::FillBuffer - Multi perform failed with code %d", result);
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
// CCurlFile implementation
//------------------------------------------------------------------------------

CCurlFile::CCurlFile()
{
    m_opened = false;
    m_seekable = true;
    m_multisession = true;
    m_bufferSize = 32768;
    m_httpresponse = -1;
    m_state = new CReadState();
}

CCurlFile::~CCurlFile()
{
    Close();
    g_curlSessionPool.easy_release(&m_state->m_easyHandle, &m_state->m_multiHandle);
    delete m_state;
}

void CCurlFile::SetCommonOptions(CReadState* state)
{
    CURL_HANDLE* h = state->m_easyHandle;

    curl_easy_reset(h);

    curl_easy_setopt(h, CURLOPT_WRITEDATA, state);
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, xbmc_write_cb);

    curl_easy_setopt(h, CURLOPT_WRITEHEADER, state);
    curl_easy_setopt(h, CURLOPT_HEADERFUNCTION, xbmc_header_cb);
    curl_easy_setopt(h, CURLOPT_HEADER, 0L);

    // Follow redirects
    curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(h, CURLOPT_MAXREDIRS, 5L);

    // No signals (thread safety)
    curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1L);

    // Fail on HTTP errors (4xx, 5xx)
    curl_easy_setopt(h, CURLOPT_FAILONERROR, 1L);

    // No SSL verification (Xbox 360 has no cert store)
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 0L);

    // Set URL
    curl_easy_setopt(h, CURLOPT_URL, m_url.c_str());
    curl_easy_setopt(h, CURLOPT_TRANSFERTEXT, 0L);

    // Set user agent
    curl_easy_setopt(h, CURLOPT_USERAGENT, g_sysinfo.GetUserAgent().c_str());

    // Timeouts
    curl_easy_setopt(h, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(h, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(h, CURLOPT_LOW_SPEED_TIME, 20L);

#if defined(_XBOX)
    // Apply Xbox 360 XNet socket patches to every socket libcurl creates
    curl_easy_setopt(h, CURLOPT_SOCKOPTFUNCTION, xbmc_curl_sockopt_cb);
#endif
}

bool CCurlFile::Open(const CURL& url, bool bBinary)
{
    if (!g_application.getNetwork().IsAvailable())
        return false;

    m_opened = true;

    // Build the URL string
    m_url = url.Get();

    ::CURL url2(url);

    CLog::Log(LOGDEBUG, "CCurlFile::Open(%p) %s", (void*)this, m_url.c_str());

    if (m_state->m_easyHandle == NULL)
        g_curlSessionPool.easy_aquire(url2.GetProtocol(), url2.GetHostName(),
                                      &m_state->m_easyHandle, &m_state->m_multiHandle);

    if (!m_state->m_easyHandle)
    {
        CLog::Log(LOGERROR, "CCurlFile::Open - Failed to init curl easy handle");
        m_opened = false;
        return false;
    }

    SetCommonOptions(m_state);
    m_state->m_sendRange = m_seekable;

    m_httpresponse = m_state->Connect(m_bufferSize);
    if (m_httpresponse < 0 || m_httpresponse >= 400)
    {
        // If https failed, fallback to http
        if (url2.GetProtocol().Equals("https"))
        {
            CLog::Log(LOGWARNING, "CCurlFile::Open - HTTPS failed for %s, falling back to HTTP", m_url.c_str());

            m_state->Disconnect();
            g_curlSessionPool.easy_release(&m_state->m_easyHandle, &m_state->m_multiHandle);

            m_url.Replace("https://", "http://");
            url2 = ::CURL(m_url);

            g_curlSessionPool.easy_aquire(url2.GetProtocol(), url2.GetHostName(),
                                          &m_state->m_easyHandle, &m_state->m_multiHandle);

            SetCommonOptions(m_state);
            m_state->m_sendRange = m_seekable;

            m_httpresponse = m_state->Connect(m_bufferSize);
        }

        if (m_httpresponse < 0 || m_httpresponse >= 400)
        {
            CLog::Log(LOGERROR, "CCurlFile::Open - Connect returned %d for %s", m_httpresponse, m_url.c_str());
            m_opened = false;
            return false;
        }
    }

    // Only enable multi-session for HTTP(S)
    m_multisession = false;
    if (url2.GetProtocol().Equals("http") || url2.GetProtocol().Equals("https"))
    {
        m_multisession = true;
        if (m_state->m_httpheader.GetValue("Server").Find("Portable SDK for UPnP devices") >= 0)
        {
            CLog::Log(LOGWARNING, "CCurlFile::Open - Disabling multi session due to broken libupnp server");
            m_multisession = false;
        }
    }

    // If chunked transfer encoding, we don't know the real size
    if (m_state->m_httpheader.GetValue("Transfer-Encoding").Equals("chunked"))
        m_state->m_fileSize = 0;

    // Determine seekability
    m_seekable = false;
    if (m_state->m_fileSize > 0)
    {
        m_seekable = true;

        // If server explicitly says it can't seek, respect that
        if (m_state->m_httpheader.GetValue("Accept-Ranges").Equals("none"))
            m_seekable = false;
    }

    // Get the effective URL (may have been redirected)
    char* efurl;
    if (CURLE_OK == curl_easy_getinfo(m_state->m_easyHandle, CURLINFO_EFFECTIVE_URL, &efurl) && efurl)
        m_url = efurl;

    return true;
}

unsigned int CCurlFile::Read(void* lpBuf, int64_t uiBufSize)
{
    if (!m_opened)
        return 0;

    return m_state->Read(lpBuf, uiBufSize);
}

__int64 CCurlFile::Seek(__int64 iFilePosition, int iWhence)
{
    if (!m_opened)
        return -1;

    int64_t nextPos = m_state->m_filePos;
    switch (iWhence)
    {
    case SEEK_SET:
        nextPos = iFilePosition;
        break;
    case SEEK_CUR:
        nextPos += iFilePosition;
        break;
    case SEEK_END:
        if (m_state->m_fileSize)
            nextPos = m_state->m_fileSize + iFilePosition;
        else
            return -1;
        break;
    case SEEK_POSSIBLE:
        return m_seekable ? 1 : 0;
    default:
        return -1;
    }

    if (m_state->m_fileSize && nextPos > m_state->m_fileSize)
        return -1;

    if (m_state->Seek(nextPos))
        return nextPos;

    if (!m_seekable)
        return -1;

    // Need to reconnect with new position
    CReadState* oldstate = NULL;
    if (m_multisession)
    {
        ::CURL url(m_url);
        oldstate = m_state;
        m_state = new CReadState();

        g_curlSessionPool.easy_aquire(url.GetProtocol(), url.GetHostName(),
                                      &m_state->m_easyHandle, &m_state->m_multiHandle);

        // setup common curl options
        SetCommonOptions(m_state);
    }
    else
    {
        m_state->Disconnect();
    }

    m_state->m_filePos = nextPos;
    m_state->m_sendRange = true;
    if (oldstate)
        m_state->m_fileSize = oldstate->m_fileSize;

    long response = m_state->Connect(m_bufferSize);
    if (response < 0 && (m_state->m_fileSize == 0 || m_state->m_fileSize != m_state->m_filePos))
    {
        m_seekable = false;
        if (oldstate)
        {
            g_curlSessionPool.easy_release(&m_state->m_easyHandle, &m_state->m_multiHandle);
            delete m_state;
            m_state = oldstate;
        }
        return -1;
    }

    if (oldstate)
    {
        g_curlSessionPool.easy_release(&oldstate->m_easyHandle, &oldstate->m_multiHandle);
        delete oldstate;
    }

    return m_state->m_filePos;
}

int64_t CCurlFile::GetLength()
{
    if (!m_opened) return 0;
    return m_state->m_fileSize;
}

int64_t CCurlFile::GetPosition()
{
    if (!m_opened) return 0;
    return m_state->m_filePos;
}

void CCurlFile::Close()
{
    if (!m_opened)
        return;

    m_state->Disconnect();
    g_curlSessionPool.easy_release(&m_state->m_easyHandle, &m_state->m_multiHandle);

    m_url.Empty();
    m_opened = false;
}

bool CCurlFile::Get(const CStdString& strURL, CStdString& strHTML)
{
    if (Open(strURL))
    {
        if (ReadData(strHTML))
        {
            Close();
            return true;
        }
    }
    Close();
    return false;
}

bool CCurlFile::ReadData(CStdString& strHTML)
{
    int size_read = 0;
    strHTML = "";
    char buffer[16384];
    while ((size_read = Read(buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[size_read] = 0;
        strHTML.append(buffer, size_read);
    }
    if (m_state->m_cancelled)
        return false;
    return true;
}

int CCurlFile::Stat(const CURL& url, struct __stat64* buffer)
{
    if (buffer)
    {
        memset(buffer, 0, sizeof(struct __stat64));
        if (m_opened)
        {
            buffer->st_size = GetLength();
            buffer->st_mode = 0x8000; // _S_IFREG
            return 0;
        }
    }
    return -1;
}

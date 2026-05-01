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
 */

#include "FileBase.h"
#include "..\utils\RingBuffer.h"
#include "..\utils\StdString.h"
#include "..\utils\HttpHeader.h"

// Forward declare curl types to avoid pulling in curl headers here
typedef void CURL_HANDLE;
typedef void CURLM;
struct curl_slist;

namespace XFILE
{

class CCurlFile : public CFileBase
{
public:
    CCurlFile();
    virtual ~CCurlFile();

    // CFileBase interface
    virtual bool Open(const CURL& url, bool bBinary = true);
    virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
    virtual bool ReadString(char *szLine, int iLineLength)     { return m_state->ReadString(szLine, iLineLength); }
    virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
    virtual int64_t GetPosition();
    virtual int64_t GetLength();
    virtual void Close();
    virtual int Stat(const CURL& url, struct __stat64* buffer);

    // Inner read state — manages the curl multi session
    class CReadState
    {
    public:
        CReadState();
        ~CReadState();

        CURL_HANDLE*    m_easyHandle;
        CURLM*          m_multiHandle;

        CRingBuffer     m_buffer;
        unsigned int    m_bufferSize;

        char*           m_overflowBuffer;
        unsigned int    m_overflowSize;
        int             m_stillRunning;
        bool            m_cancelled;
        int64_t         m_fileSize;
        int64_t         m_filePos;
        bool            m_bFirstLoop;
        bool            m_sendRange;
        bool            m_isPaused;
        bool            m_headerdone;
        char*           m_readBuffer;
        CHttpHeader     m_httpheader;

        size_t WriteCallback(char *buffer, size_t size, size_t nitems);
        size_t HeaderCallback(void *ptr, size_t size, size_t nmemb);

        bool         Seek(int64_t pos);
        unsigned int Read(void* lpBuf, int64_t uiBufSize);
        bool         ReadString(char *szLine, int iLineLength);
        bool         FillBuffer(unsigned int want);

        void         SetResume();
        long         Connect(unsigned int size);
        void         Disconnect();
    };

    // Convenience methods for simple HTTP GET
    bool Get(const CStdString& strURL, CStdString& strHTML);
    bool ReadData(CStdString& strHTML);

    void Cancel();
    void SetUserAgent(CStdString sUserAgent)                   { m_userAgent = sUserAgent; }
    void SetTimeout(int connecttimeout)                        { m_connecttimeout = connecttimeout; }

protected:
    void SetCommonOptions(CReadState* state);

    CReadState*     m_state;
    unsigned int    m_bufferSize;

    CStdString      m_url;
    CStdString      m_userAgent;
    bool            m_opened;
    bool            m_seekable;
    bool            m_multisession;
    int             m_connecttimeout;
    int             m_httpresponse;
};

}

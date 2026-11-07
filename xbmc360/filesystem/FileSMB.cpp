#include "FileSMB.h"
#include "utils\SingleLock.h"
#include "utils\Log.h"
#include <fcntl.h>
#include "XBLibSmb2.h"

using namespace XFILE;

// Shared connection manager - serializes all SMB operations via its
// CCriticalSection base class, but no longer owns individual file handles.
CXBLibSMB2 xbsmb_f;

CFileSMB::CFileSMB()
{
	m_pFileHandle = NULL;
	m_fileSize = 0;
}

CFileSMB::~CFileSMB()
{
	Close();
}

bool CFileSMB::Open(const CURL& strURL)
{
	Close(); // Ensure any previous handle is released

	CSingleLock lock(xbsmb_f);

	m_pFileHandle = xbsmb_f.FileOpen(strURL, m_fileSize);

	if(!m_pFileHandle)
	{
		CLog::Log(LOGERROR, "CFileSMB::Open - Failed to open file");
		return false;
	}

	return true;
}

bool CFileSMB::OpenForWrite(const CURL& strURL, bool bOverWrite)
{
	Close(); // Ensure any previous handle is released

	CSingleLock lock(xbsmb_f);

	m_pFileHandle = xbsmb_f.FileOpenForWrite(strURL, bOverWrite, m_fileSize);

	if(!m_pFileHandle)
	{
		CLog::Log(LOGERROR, "CFileSMB::OpenForWrite - Failed to open file");
		return false;
	}

	return true;
}

__int64 CFileSMB::GetLength()
{
	return (__int64)m_fileSize;
}

__int64 CFileSMB::GetPosition()
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.FileGetPosition(m_pFileHandle);
}

void CFileSMB::Close()
{
	if(m_pFileHandle)
	{
		CSingleLock lock(xbsmb_f);
		xbsmb_f.FileClose(m_pFileHandle);
		m_pFileHandle = NULL;
	}
	m_fileSize = 0;
}

unsigned int CFileSMB::Read(void *lpBuf, __int64 uiBufSize)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.FileRead(m_pFileHandle, lpBuf, uiBufSize);
}
	
__int64 CFileSMB::Seek(__int64 iFilePosition, int iWhence)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.FileSeek(m_pFileHandle, iFilePosition, iWhence);
}

int CFileSMB::Write(const void* lpBuf, __int64 uiBufSize)
{
	CSingleLock lock(xbsmb_f);

	int iWritten = xbsmb_f.FileWrite(m_pFileHandle, lpBuf, uiBufSize);

	// The length was read when opening: keep it up as the file grows
	__int64 iPos = xbsmb_f.FileGetPosition(m_pFileHandle);
	if(iPos > (__int64)m_fileSize)
		m_fileSize = iPos;

	return iWritten;
}

int CFileSMB::Stat(const CURL& url, struct __stat64* buffer)
{
	CSingleLock lock(xbsmb_f);

	struct smb2_stat_64 st;
	if(!xbsmb_f.Stat(url, st))
		return -1;

	if(buffer)
	{
		memset(buffer, 0, sizeof(struct __stat64));
		buffer->st_mode = (st.smb2_type == SMB2_TYPE_DIRECTORY) ? _S_IFDIR : _S_IFREG;
		buffer->st_nlink = 1;
		buffer->st_size = st.smb2_size;
		buffer->st_atime = st.smb2_atime;
		buffer->st_mtime = st.smb2_mtime;
		buffer->st_ctime = st.smb2_ctime;
	}

	return 0;
}

bool CFileSMB::Exists(const CURL& url)
{
	CSingleLock lock(xbsmb_f);

	struct smb2_stat_64 st;
	return xbsmb_f.Stat(url, st);
}

bool CFileSMB::Delete(const CURL& url)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.Delete(url);
}

bool CFileSMB::Rename(const CURL& url, const CURL& urlnew)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.Rename(url, urlnew);
}
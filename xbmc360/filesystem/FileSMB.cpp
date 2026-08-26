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
	CLog::Log(LOGERROR, "CFileSMB OpenForWrite not yet supported.");
	return false; // TODO!!
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
	// TODO!
	return 0;
}

int CFileSMB::Stat(const CURL& url, struct __stat64* buffer)
{
	// TODO!!
	return 0;
}

bool CFileSMB::Exists(const CStdString& strPath)
{
	// TODO!!
	return false;
}
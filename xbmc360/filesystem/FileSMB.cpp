#include "FileSMB.h"
#include "utils\SingleLock.h"
#include "utils\log.h"
#include <fcntl.h>

using namespace XFILE;

CXBLibSMB2 xbsmb_f;

CFileSMB::CFileSMB()
{

}

CFileSMB::~CFileSMB()
{

}

bool CFileSMB::Open(const CURL& strURL, bool bBinary)
{
	// LibSMB2 isn't thread safe, always lock
	CSingleLock lock(xbsmb_f);

	xbsmb_f.Init();

	xbsmb_f.OpenFile(strURL);

	return true;
}

bool CFileSMB::OpenDir(const CURL& strURL, bool bIsOk)
{
	CSingleLock lock(xbsmb_f);

	try
	{
		xbsmb_f.Init();
		xbsmb_f.OpenDir(strURL);
		return bIsOk = true;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "Failed To Open The Following Directory: %s", strURL);
		return bIsOk = false;
	}
}

bool CFileSMB::OpenForWrite(const CURL& strURL, bool bOverWrite)
{
	CSingleLock lock(xbsmb_f);
	void *lpBuf;
	__int64 uiBufSize;
	try
	{
		xbsmb_f.Init();
		xbsmb_f.OpenFile(strURL);
		xbsmb_f.Write(static_cast<uint8_t*>(lpBuf), (unsigned int)uiBufSize);
		return bOverWrite = true;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "Failed to Open to write");
		return bOverWrite = false;
	}
	return false; 
}

__int64 CFileSMB::GetLength()
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.GetLength();
}

__int64 CFileSMB::GetPosition()
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.GetPosition();
}

void CFileSMB::Close()
{
	CSingleLock lock(xbsmb_f);

	xbsmb_f.Close();
}

unsigned int CFileSMB::Read(void *lpBuf, __int64 uiBufSize)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.Read(lpBuf, uiBufSize);
}
	
__int64 CFileSMB::Seek(__int64 iFilePosition, int iWhence)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.Seek(iFilePosition, iWhence);
}

int CFileSMB::Write(void* lpBuf, __int64 uiBufSize)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.Write(lpBuf, uiBufSize);
}

int CFileSMB::Stat(const CURL& url, struct __stat64* buffer)
{
	CSingleLock lock(xbsmb_f);

	return xbsmb_f.Stat(url);
}

bool CFileSMB::Exists(const CStdString& strPath)
{	
        CSingleLock lock(xbsmb_f);
	
        CURL url;
	FILE *file = fopen(strPath.c_str(), "rb");
	
	if(file != NULL)
	{
		fclose(file);
		return true;
	}
	
	else
	{
		xbsmb_f.OpenDir(url.GetFileName());
		return false;
	}
	return true;
}

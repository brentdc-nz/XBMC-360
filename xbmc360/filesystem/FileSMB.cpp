#include "FileSMB.h"
#include "utils\SingleLock.h"
#include "utils\Log.h"
#include <fcntl.h>
#include "libraries\libsmb2\XBLibSmb2.h"

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

bool CFileSMB::OpenForWrite(const CURL& strURL, bool bOverWrite)
{
	CLog::Log(LOGERROR, "CFileSMB OpenForWrite not yet supported.");
	return false; // TODO!!
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
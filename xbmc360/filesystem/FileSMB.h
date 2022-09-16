#ifndef H_CFILESMB
#define H_CFILESMB

#include "FileBase.h"
#include "utils\CriticalSection.h"
#include "libraries\libsmb2\XBLibSmb2.h"

#include "..\libraries\libsmb2\libsmb2.h"
#include "..\libraries\libsmb2\smb2.h"

namespace XFILE
{

class CFileSMB : public CFileBase
{
public:
	CFileSMB();
	~CFileSMB();

	virtual __int64 GetPosition();
	virtual __int64 GetLength();

	virtual bool Open(const CURL& strURL, bool bBinary = true);
	virtual bool OpenDir(const CURL& strURL, bool bBinary = true);
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false);
	virtual void Close();
	virtual unsigned int Read(void* lpBuf, __int64 uiBufSize);
	virtual int Write(void* lpBuf, __int64 uiBufSize);
	virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	virtual int Stat(const CURL& url, struct __stat64* buffer);
	static bool Exists(const CStdString& strPath);
};

}

#endif //H_CFILESMB

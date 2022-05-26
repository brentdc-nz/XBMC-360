#ifndef H_CFILEHD
#define H_CFILEHD

#include "FileBase.h"
#include "..\utils\StdString.h"
#include "..\utils\AutoPtrHandle.h"

namespace XFILE
{

class CFileHD : public CFileBase
{
public:
	CFileHD();
	~CFileHD();

	virtual __int64 GetPosition();
	virtual __int64 GetLength();
	virtual bool Open(const CURL& strURL, bool bBinary = true);
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false);
	virtual void Close();
	virtual unsigned int Read(void* lpBuf, __int64 uiBufSize);
	virtual int Write(const void* lpBuf, __int64 uiBufSize);
	virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	static bool Exists(const CStdString& strPath);
	virtual int Stat(const CURL& url, struct __stat64* buffer);

protected:
	CStdString GetLocal(const CURL &url);
	AUTOPTR::CAutoPtrHandle m_hFile;
	__int64 m_i64FileLength;
	__int64 m_i64FilePos;
};

};

#endif //H_CFILEHD
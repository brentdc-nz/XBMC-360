#ifndef H_CFILEHD
#define H_CFILEHD

#include "IFile.h"
#include "..\utils\StdString.h"
#include "..\utils\AutoPtrHandle.h"

namespace XFILE
{

class CFileHD : public IFile
{
public:
	CFileHD();
	~CFileHD();

	virtual __int64 GetPosition();
	virtual __int64 GetLength();
	virtual bool Open(const CURL& strURL);
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false);
	virtual void Close();
	virtual unsigned int Read(void* lpBuf, __int64 uiBufSize);
	virtual int Write(const void* lpBuf, __int64 uiBufSize);
	virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	virtual bool Exists(const CURL& url);
	virtual bool Delete(const CURL& url);
	virtual bool Rename(const CURL& url, const CURL& urlnew);
	virtual int Stat(const CURL& url, struct __stat64* buffer);

protected:
	CStdString GetLocal(const CURL &url);
	AUTOPTR::CAutoPtrHandle m_hFile;
	__int64 m_i64FileLength;
	__int64 m_i64FilePos;
};

};

#endif //H_CFILEHD
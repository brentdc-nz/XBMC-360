#ifndef H_CFILEFTP
#define H_CFILEFTP

#include "FileBase.h"
#include "utils\CriticalSection.h"
#include "..\services\FTPServer\FTPFile.h"

namespace XFILE
{
class CFileFTP : public CFileBase
{
public:
	// Standards from CFilebase	
	CFileFTP();
	~CFileFTP();

	virtual int64_t GetPosition();
	virtual int64_t GetLength();

	virtual bool Open(const CURL& strURL, bool bBinary = true);
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false);
	virtual void Close();
	virtual unsigned int Read(void* lpBuf, __int64 uiBufSize);
	virtual int Write(const void* lpBuf, __int64 uiBufSize);
	virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	virtual int Stat(const CURL& url, struct __stat64* buffer);
private:
	FILE *m_pFile;
};

};

#endif //H_CFILEFTP
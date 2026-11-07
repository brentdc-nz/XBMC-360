#ifndef H_CFILESMB
#define H_CFILESMB

#include "IFile.h"
#include "..\utils\CriticalSection.h"

#include "libsmb2.h"
#include "smb2.h"

class CXBLibSMB2;

namespace XFILE
{

class CFileSMB : public IFile
{
public:
	CFileSMB();
	~CFileSMB();

	virtual __int64 GetPosition();
	virtual __int64 GetLength();

	virtual bool Open(const CURL& strURL);
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false);
	virtual void Close();
	virtual unsigned int Read(void* lpBuf, __int64 uiBufSize);
	virtual int Write(const void* lpBuf, __int64 uiBufSize);
	virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	virtual int Stat(const CURL& url, struct __stat64* buffer);
	virtual bool Exists(const CURL& url);
	virtual bool Delete(const CURL& url);
	virtual bool Rename(const CURL& url, const CURL& urlnew);

protected:
	struct smb2fh*	m_pFileHandle;
	UINT64			m_fileSize;
};

}

#endif //H_CFILESMB

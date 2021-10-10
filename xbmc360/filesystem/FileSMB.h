#ifndef H_CFILESMB
#define H_CFILESMB

#include "FileBase.h"
#include "..\utils\CriticalSection.h"

#include "..\libraries\libsmb2\libsmb2.h"
#include "..\libraries\libsmb2\smb2.h"

namespace XFILE
{

class CLibSMB2Wrap : public CCriticalSection
{
public:
	CLibSMB2Wrap();
	~CLibSMB2Wrap();

	void Init();
	bool OpenFile();
	__int64 Seek(__int64 iFilePosition, int iWhence);
	unsigned int Read(void *lpBuf, __int64 uiBufSize);
	__int64 GetLength();
	__int64 GetPosition();
	void Close();

private:
	struct smb2_context*	m_pLibSMB2Context;
	struct smb2fh*			m_pLibSMB2FH;
	struct smb2_url*		m_pUrl;
	__int64 m_iFileSize;
};



class CFileSMB : public CFileBase
{
public:
	CFileSMB();
	~CFileSMB();

	virtual __int64 GetPosition();
	virtual __int64 GetLength();

	virtual bool Open(const CURL& strURL, bool bBinary = true);
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false);
	virtual void Close();
	virtual unsigned int Read(void* lpBuf, __int64 uiBufSize);
	virtual int Write(const void* lpBuf, __int64 uiBufSize);
	virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	virtual int Stat(const CURL& url, struct __stat64* buffer);
	static bool Exists(const CStdString& strPath);

protected:

};

}

#endif //H_CFILESMB

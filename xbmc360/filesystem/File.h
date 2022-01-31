#ifndef H_CFILE
#define H_CFILE

#include "FileBase.h"
#include "..\utils\StdString.h"

// Indicate that caller can handle truncated reads, where function returns before entire buffer has been filled
#define READ_TRUNCATED 0x1

namespace XFILE
{

class CFile
{
public:
	CFile(){};
	virtual ~CFile(){};

	bool Open(const CStdString& strURLFile, unsigned int iFlags = 0);
	bool OpenForWrite(const CStdString& strFileName, bool bOverWrite = false);
	unsigned int Read(void* lpBuf, unsigned int uiBufSize, unsigned int flags);
	int Write(const void* lpBuf, int64_t uiBufSize);
	__int64 GetPosition();
	__int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	__int64 GetLength();
	void Close();

	static bool Exists(const CStdString& strFileName);
	static int  Stat(const CStdString& strFileName, struct __stat64* buffer);

private:
	CFileBase* m_pFile;
	unsigned int m_iFlags;
};

};
#endif //H_CFILE
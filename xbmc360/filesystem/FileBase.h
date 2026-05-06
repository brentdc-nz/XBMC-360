#ifndef H_CFILEBASE
#define H_CFILEBASE

#include "..\utils\StdString.h"
#include "..\URL.h"

#define SEEK_POSSIBLE 0x10 // flag used to check if protocol allows seeks

namespace XFILE
{

class CFileBase
{
public:
	CFileBase();
	virtual ~CFileBase();

	virtual bool Open(const CURL& strURL, bool bBinary = true) = 0;
	virtual bool Exists(const CURL& url) { return false; }
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false) { return false; };
	virtual int Stat(const CURL& url, struct __stat64* buffer) = 0;/*
	virtual int Stat(struct __stat64* buffer) = 0;*/
	virtual unsigned int Read(void* lpBuf, int64_t uiBufSize) = 0;
	virtual int Write(const void* lpBuf, int64_t uiBufSize) { return -1;};/*
  virtual bool ReadString(char *szLine, int iLineLength);*/
	virtual __int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET) = 0;
	virtual void Close() = 0;
	virtual int64_t GetPosition() = 0;
	virtual int64_t GetLength() = 0;/*
  virtual void Flush() { }
*/
	/* Returns the minium size that can be read from input stream.   *
	* For example cdrom access where access could be sector based.   *
	* This will cause file system to buffer read requests, to        *
	* to meet the requirement of CFile.                              *
	* It can also be used to indicate a file system is non buffered  *
	* but accepts any read size, have it return the value 1          */
	virtual int  GetChunkSize() { return 0; }

	virtual bool Delete(const CURL& url) { return false; }
	virtual bool Rename(const CURL& url, const CURL& urlnew) { return false; }
};

class CRedirectException
{
public:
	CFileBase *m_pNewFileImp;
	CURL      *m_pNewUrl;

	CRedirectException() : m_pNewFileImp(NULL), m_pNewUrl(NULL) { }

	CRedirectException(CFileBase *pNewFileImp, CURL *pNewUrl = NULL) 
		: m_pNewFileImp(pNewFileImp)
		, m_pNewUrl(pNewUrl)
	{ }
};

}

#endif //H_CFILEBASE
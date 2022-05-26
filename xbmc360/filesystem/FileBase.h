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
	virtual bool OpenForWrite(const CURL& strURL, bool bOverWrite = false) { return false; };/*
  virtual bool Exists(const CURL& url) = 0;*/
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

 /* virtual bool SkipNext(){return false;}

  virtual bool Delete(const CURL& url) { return false; }
  virtual bool Rename(const CURL& url, const CURL& urlnew) { return false; }
  virtual bool SetHidden(const CURL& url, bool hidden) { return false; }

  virtual int IoControl(EIoControl request, void* param) { return -1; }

  virtual CStdString GetContent()                            { return "application/octet-stream"; }*/
};

}

#endif //H_CFILEBASE
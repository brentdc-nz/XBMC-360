//////////////////////////////////////////////////////////////////////
// IFile.h: interface for the IFile class
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_)
#define AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_

#pragma once

#include "URL.h"

#include <stdint.h>
#include <sys/stat.h>

namespace XFILE
{

struct SNativeIoControl
{
	int   request;
	void* param;
};

struct SCacheStatus
{
	int64_t forward;  /**< number of bytes cached forward of current position */
	unsigned maxrate;  /**< maximum number of bytes per second cache is allowed to fill */
	unsigned currate;  /**< average read rate from source file since last position change */
	bool     full;  /**< is the cache full */
};

typedef enum {
	IOCTRL_NATIVE        = 1, /**< SNativeIoControl structure, containing what should be passed to native ioctrl */
	IOCTRL_SEEK_POSSIBLE = 2, /**< return 0 if known not to work, 1 if it should work */
	IOCTRL_CACHE_STATUS  = 3, /**< SCacheStatus structure */
	IOCTRL_CACHE_SETRATE = 4, /**< unsigned int with speed limit for caching in bytes per second */
} EIoControl;

class IFile
{
public:
	IFile();
	virtual ~IFile();

	virtual bool Open(const CURL& url) = 0;
	virtual bool OpenForWrite(const CURL& url, bool bOverWrite = false) { return false; };
	virtual bool Exists(const CURL& url) { return false; };
	virtual int Stat(const CURL& url, struct __stat64* buffer) = 0;
	virtual int Stat(struct __stat64* buffer);
	virtual unsigned int Read(void* lpBuf, int64_t uiBufSize) = 0;
	virtual int Write(const void* lpBuf, int64_t uiBufSize) { return -1;};
	virtual bool ReadString(char *szLine, int iLineLength);
	virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET) = 0;
	virtual void Close() = 0;
	virtual int64_t GetPosition() = 0;
	virtual int64_t GetLength() = 0;
	virtual void Flush() { }

	/* Returns the minium size that can be read from input stream.   *
	* For example cdrom access where access could be sector based.  *
	* This will cause file system to buffer read requests, to       *
	* to meet the requirement of CFile.                             *
	* It can also be used to indicate a file system is non buffered *
	* but accepts any read size, have it return the value 1         */
	virtual int  GetChunkSize() {return 0;}

	virtual bool SkipNext(){return false;}

	virtual bool Delete(const CURL& url) { return false; }
	virtual bool Rename(const CURL& url, const CURL& urlnew) { return false; }
	virtual bool SetHidden(const CURL& url, bool hidden) { return false; }

	virtual int IoControl(EIoControl request, void* param) { return -1; }

	virtual CStdString GetContent()                            { return "application/octet-stream"; }
};

class CRedirectException
{
public:
	IFile *m_pNewFileImp;
	CURL  *m_pNewUrl;

	CRedirectException() : m_pNewFileImp(NULL), m_pNewUrl(NULL) { }

	CRedirectException(IFile *pNewFileImp, CURL *pNewUrl=NULL)
	: m_pNewFileImp(pNewFileImp)
	, m_pNewUrl(pNewUrl)
	{ }
};

}

#endif // !defined(AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_)
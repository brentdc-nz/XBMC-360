#include "FileFTP.h"
#include "utils\log.h"

#include <fcntl.h>

using namespace XFILE;

CFTPFileWriter *xbftp_w; //Write 
CFTPFileReader *xbftp_r; //Read

CFileFTP::CFileFTP()
{
}

CFileFTP::~CFileFTP()
{
}

bool CFileFTP::Open(const CURL &strURL, bool bBinarry)
{
	//Only Open for read
	if(bBinarry == 0)
	{
		xbftp_r->OpenFile(strURL.Get());
		xbftp_r->Process();
		return false;
	}

	//Only Open For Write!!!
	else if(bBinarry == 1)
	{
		xbftp_w->Open(strURL.Get());
		xbftp_w->Process();
	    return true;
	}

	else
	{
		CLog::Log(LOGERROR, "Invalid argument to read");	
	}
	return bBinarry; 
}

bool CFileFTP::OpenForWrite(const CURL& strURL, bool bOverWrite)
{
	
	xbftp_w->Open(strURL.Get());
	xbftp_w->Process();
	
	return true; //TODO
}

int CFileFTP::Stat(const CURL &strURL, struct __stat64* buffer)
{
	return 0; //TODO
}

unsigned int CFileFTP::Read(void* lpBuf, int64_t uiBufSize)
{
    xbftp_r->Process();
	return NULL;
}

int CFileFTP::Write(const void* lpBuf, int64_t uiBufSize)
{
	xbftp_w->Process();
	return NULL;
}

__int64 CFileFTP::Seek(__int64 iFilePosition, int iWhence)
{
	if(iFilePosition == NULL)
	{
		return 0;
	}
	else
	{
		fseek(m_pFile, NULL, iWhence);	
	}
	
	return 0;
}

int64_t CFileFTP::GetLength()
{
  return 0; //WIP
}

int64_t CFileFTP::GetPosition()
{
	return 0; //WIP
}

void CFileFTP::Close()
{
	xbftp_w->CloseWrite();
	xbftp_r->CloseRead();
}

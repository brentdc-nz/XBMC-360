#include "FileFTP.h"
#include "utils\log.h"

#include <fcntl.h>

using namespace XFILE;
CFTPFileWriter *xbftp_f;

CFileFTP::CFileFTP()
{
}

CFileFTP::~CFileFTP()
{
}

bool CFileFTP::Open(const CURL &strURL, bool bBinarry)
{
	xbftp_f->Process();
	return false; //TODO
}

bool CFileFTP::OpenForWrite(const CURL& strURL, bool bOverWrite)
{
	return false; //TODO
}

int CFileFTP::Stat(const CURL &strURL, struct __stat64* buffer)
{
	return 0; //TODO
}

unsigned int CFileFTP::Read(void* lpBuf, int64_t uiBufSize)
{
	return 0; //TODO
}

int CFileFTP::Write(const void* lpBuf, int64_t uiBufSize)
{
	return 0; //TODO
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
	return 0;
}

int64_t CFileFTP::GetPosition()
{
	return 0; //WIP
}

void CFileFTP::Close()
{
	xbftp_f->Close();
}

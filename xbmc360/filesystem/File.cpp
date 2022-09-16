#include "File.h"
#include "FileHD.h"
#include "FileFactory.h"
#include "FileSMB.h"
#include "utils\Log.h"
#include "utils\Stdafx.h"
#include "utils\AutoPtrHandle.h"

using namespace XFILE;

CFile::CFile()
{
	m_pFile = NULL;
//	m_pBuffer = NULL;
//	m_flags = 0;
	m_bitStreamStats = NULL;
}

CFile::~CFile()
{
	if(m_pFile)
		SAFE_DELETE(m_pFile);

//	if(m_pBuffer)
//		SAFE_DELETE(m_pBuffer);

	if(m_bitStreamStats)
		SAFE_DELETE(m_bitStreamStats);
}

bool CFile::Open(const CStdString& strURLFile, unsigned int iFlags)
{
	m_iFlags = iFlags;
	try
	{
		m_pFile = CFileFactory::CreateLoader(strURLFile);
		if (m_pFile)
			return m_pFile->Open(strURLFile);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "File::Open - unknown exception when opening %s", strURLFile.c_str());
		SAFE_DELETE(m_pFile);
		return false;
    }

	return false;
}

bool CFile::OpenForWrite(const CStdString& strFileName, bool bOverWrite)
{
	try
	{
		CURL url(strFileName);

		m_pFile = CFileFactory::CreateLoader(url);

		if(m_pFile && m_pFile->OpenForWrite(url, bOverWrite))
		{
			return true;
		}
		return false;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception opening %s", __FUNCTION__, strFileName.c_str());
	}

	CLog::Log(LOGERROR, "%s - Error opening %s", __FUNCTION__, strFileName.c_str());
	return false;
}

unsigned int CFile::Read(void *lpBuf, unsigned int uiBufSize, unsigned flags)
{

    if (m_pFile) 
    {
      if(flags & READ_TRUNCATED)
        return m_pFile->Read(lpBuf, uiBufSize);
      else
      {        
        unsigned int done = 0;
        while((uiBufSize-done) > 0)
        {
          unsigned int curr = m_pFile->Read((char*)lpBuf+done, uiBufSize-done);
          if(curr==0)
            break;

          done+=curr;
        }
        return done;
      }        
    }
    return 0;
}

int CFile::Write(const void* lpBuf, int64_t uiBufSize)
{
	try
	{
		return m_pFile->Write(lpBuf, uiBufSize);
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	return -1;
}

__int64 CFile::Seek(__int64 iFilePosition, int iWhence)
{
    if (m_pFile) return m_pFile->Seek(iFilePosition, iWhence);
    return 0;
}

__int64 CFile::GetLength()
{
	if(m_pFile) return m_pFile->GetLength();
	
	return 0;
}

__int64 CFile::GetPosition()
{
    if(m_pFile) return m_pFile->GetPosition();

    return 0;
}

void CFile::Close()
{
	try
	{
		if (m_pFile)
			m_pFile->Close();

		SAFE_DELETE(m_pFile);
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	return;
}

bool CFile::Exists(const CStdString& strFileName)
{
	if(strFileName.IsEmpty()) return false;
		
	//
	// TODO - Check other future protocols i.e. ftp, http, etc
	//

	//Check HDD
	if(CFileHD::Exists(strFileName)) return true;

	//Check Samba(SMB)
	if(CFileSMB::Exists(strFileName)) return true;
	
	//TODO: if(CFileFTP::Exists(strFileName)) return true;

	CLog::Log(LOGERROR, "%s - Error checking for %s", __FUNCTION__, strFileName.c_str());
	return false;
}

int CFile::Stat(const CStdString& strFileName, struct __stat64* buffer)
{
	CURL url;

	try
	{
		CFileBase* pFile = CFileFactory::CreateLoader(url);

		int iResult = 0;

		if(pFile)
			iResult = pFile->Stat(url, buffer);

		SAFE_DELETE(pFile);
		return iResult;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}

	CLog::Log(LOGERROR, "%s - Error statting %s", __FUNCTION__, strFileName.c_str());
	return -1;
}
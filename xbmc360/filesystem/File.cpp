#include "File.h"
#include "FileHD.h"
#include "FileFactory.h"
#include "..\utils\Log.h"
#include "..\utils\Stdafx.h"
#include "..\utils\AutoPtrHandle.h"
#include "..\utils\Win32Exception.h"
#include <algorithm>
#include <memory>

using namespace XFILE;

CFile::CFile()
{
	m_pFile = NULL;
	m_pBuffer = NULL;
	m_flags = 0;
	m_bitStreamStats = NULL;
}

CFile::~CFile()
{
	if(m_pFile)
		SAFE_DELETE(m_pFile);

	if(m_pBuffer)
		SAFE_DELETE(m_pBuffer);

	if(m_bitStreamStats)
		SAFE_DELETE(m_bitStreamStats);
}

bool CFile::Open(const CStdString& strURLFile, unsigned int flags)
{
	m_flags = flags;
	try
	{
		m_pFile = CFileFactory::CreateLoader(strURLFile);
		if (m_pFile)
		{
			CURL url(strURLFile);
			if (!m_pFile->Open(url))
			{
				SAFE_DELETE(m_pFile);
				return false;
			}
		}
	}
	catch (CRedirectException *pRedirectEx)
	{
		CLog::Log(LOGDEBUG, "File::Open - redirecting implementation for %s", strURLFile.c_str());
		SAFE_DELETE(m_pFile);
		if (pRedirectEx && pRedirectEx->m_pNewFileImp)
		{
			std::auto_ptr<CURL> pNewUrl(pRedirectEx->m_pNewUrl);
			m_pFile = pRedirectEx->m_pNewFileImp;
			delete pRedirectEx;

			if (pNewUrl.get())
			{
				if (!m_pFile->Open(*pNewUrl))
				{
					SAFE_DELETE(m_pFile);
					return false;
				}
			}
			else
			{
				CURL url(strURLFile);
				if (!m_pFile->Open(url))
				{
					SAFE_DELETE(m_pFile);
					return false;
				}
			}
		}
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "File::Open - unknown exception when opening %s", strURLFile.c_str());
		SAFE_DELETE(m_pFile);
		return false;
	}

	if (m_pFile && m_pFile->GetChunkSize() && !(m_flags & READ_CHUNKED))
	{
		m_pBuffer = new CFileStreamBuffer(0);
		m_pBuffer->Attach(m_pFile);
	}

	if (m_flags & READ_BITRATE)
	{
		m_bitStreamStats = new BitstreamStats();
		m_bitStreamStats->Start();
	}

	if (m_pFile)
		return true;

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

unsigned int CFile::Read(void *lpBuf, int64_t uiBufSize)
{
    if (!m_pFile)
      return 0;

    if(m_pBuffer)
    {
      if(m_flags & READ_TRUNCATED)
      {
        unsigned int nBytes = (unsigned int)m_pBuffer->sgetn(
          (char *)lpBuf, std::min<std::streamsize>((std::streamsize)uiBufSize,
                                                    m_pBuffer->in_avail()));
        if (m_bitStreamStats && nBytes>0)
          m_bitStreamStats->AddSampleBytes(nBytes);
        return nBytes;
      }
      else
      {
        unsigned int nBytes = (unsigned int)m_pBuffer->sgetn((char*)lpBuf, uiBufSize);
        if (m_bitStreamStats && nBytes>0)
          m_bitStreamStats->AddSampleBytes(nBytes);
        return nBytes;
      }
    }

    try
    {
      if(m_flags & READ_TRUNCATED)
      {
        unsigned int nBytes = m_pFile->Read(lpBuf, uiBufSize);
        if (m_bitStreamStats && nBytes>0)
          m_bitStreamStats->AddSampleBytes(nBytes);
        return nBytes;
      }
      else
      {
        unsigned int done = 0;
        while((uiBufSize-done) > 0)
        {
          int curr = m_pFile->Read((char*)lpBuf+done, uiBufSize-done);
          if(curr<=0)
            break;

          done+=curr;
        }
        if (m_bitStreamStats && done > 0)
          m_bitStreamStats->AddSampleBytes(done);
        return done;
      }
    }
    catch(...)
    {
      CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
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

		SAFE_DELETE(m_pBuffer);
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

	// Check HDD
	if(CFileHD::Exists(strFileName)) return true;

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

bool CFile::Cache(const CStdString& strFileName, const CStdString& strDest)
{
	CFile file;
	if (file.Open(strFileName))
	{
		CFile destFile;
		if (destFile.OpenForWrite(strDest, true))
		{
			char buf[16384];
			unsigned int read;
			while ((read = file.Read(buf, sizeof(buf))) > 0)
			{
				if (destFile.Write(buf, read) < 0)
				{
					file.Close();
					destFile.Close();
					return false;
				}
			}
			destFile.Close();
			file.Close();
			return true;
		}
		file.Close();
	}
	return false;
}

//*********************************************************************************************
//*************** Stream IO for CFile objects *************************************************
//*********************************************************************************************
CFileStreamBuffer::~CFileStreamBuffer()
{
	sync();
	Detach();
}

CFileStreamBuffer::CFileStreamBuffer(int backsize)
	: std::streambuf()
	, m_file(NULL)
	, m_buffer(NULL)
	, m_backsize(backsize)
	, m_frontsize(0)
{
}

void CFileStreamBuffer::Attach(CFileBase *file)
{
	m_file = file;

	m_frontsize = CFile::GetChunkSize(m_file->GetChunkSize(), 64*1024);

	m_buffer = new char[m_frontsize+m_backsize];
	setg(0,0,0);
	setp(0,0);
}

void CFileStreamBuffer::Detach()
{
	setg(0,0,0);
	setp(0,0);
	delete[] m_buffer;
	m_buffer = NULL;
}

CFileStreamBuffer::int_type CFileStreamBuffer::underflow()
{
	if(gptr() < egptr())
		return traits_type::to_int_type(*gptr());

	if(!m_file)
		return traits_type::eof();

	size_t backsize = 0;
	if(m_backsize)
	{
		backsize = (size_t)std::min<ptrdiff_t>((ptrdiff_t)m_backsize, egptr()-eback());
		memmove(m_buffer, egptr()-backsize, backsize);
	}

	unsigned int size = 0;
	try
	{
		size = m_file->Read(m_buffer+backsize, m_frontsize);
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}

	if(size == 0)
		return traits_type::eof();

	setg(m_buffer, m_buffer+backsize, m_buffer+backsize+size);
	return traits_type::to_int_type(*gptr());
}

CFileStreamBuffer::pos_type CFileStreamBuffer::seekoff(
	off_type offset,
	std::ios_base::seekdir way,
	std::ios_base::openmode mode)
{
	// calculate relative offset
	off_type pos  = m_file->GetPosition() - (egptr() - gptr());
	off_type offset2;
	if(way == std::ios_base::cur)
		offset2 = offset;
	else if(way == std::ios_base::beg)
		offset2 = offset - pos;
	else if(way == std::ios_base::end)
		offset2 = offset + m_file->GetLength() - pos;
	else
		return std::streampos(-1);

	// a non seek shouldn't modify our buffer
	if(offset2 == 0)
		return pos;

	// try to seek within buffer
	if(gptr()+offset2 >= eback() && gptr()+offset2 < egptr())
	{
		gbump((int)offset2);
		return pos + offset2;
	}

	// reset our buffer pointer, will
	// start buffering on next read
	setg(0,0,0);
	setp(0,0);

	int64_t position = -1;
	try
	{
		if(way == std::ios_base::cur)
			position = m_file->Seek(offset, SEEK_CUR);
		else if(way == std::ios_base::end)
			position = m_file->Seek(offset, SEEK_END);
		else
			position = m_file->Seek(offset, SEEK_SET);
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
		return std::streampos(-1);
	}

	if(position<0)
		return std::streampos(-1);

	return position;
}

CFileStreamBuffer::pos_type CFileStreamBuffer::seekpos(
	pos_type pos,
	std::ios_base::openmode mode)
{
	return seekoff(pos, std::ios_base::beg, mode);
}

std::streamsize CFileStreamBuffer::showmanyc()
{
	underflow();
	return egptr() - gptr();
}
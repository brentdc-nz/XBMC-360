#include "File.h"
#include "FileFactory.h"
#include "..\utils\Log.h"
#include "..\utils\Stdafx.h"
#include "..\utils\AutoPtrHandle.h"
#include "..\utils\Win32Exception.h"
#include "..\utils\Stopwatch.h"
#include "..\URL.h"
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
	try
	{
		if (strFileName.IsEmpty())
			return false;

		CURL url(strFileName);

		std::auto_ptr<IFile> pFile(CFileFactory::CreateLoader(url));
		if (!pFile.get())
			return false;

		return pFile->Exists(url);
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	return false;
}

bool CFile::Delete(const CStdString& strFileName)
{
	try
	{
		CURL url(strFileName);

		std::auto_ptr<IFile> pFile(CFileFactory::CreateLoader(url));
		if (!pFile.get())
			return false;

		if (pFile->Delete(url))
			return true;
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	if (Exists(strFileName))
		CLog::Log(LOGERROR, "%s - Error deleting file %s", __FUNCTION__, strFileName.c_str());
	return false;
}

bool CFile::Rename(const CStdString& strFile, const CStdString& strNewFile)
{
	try
	{
		CURL url(strFile);
		CURL urlnew(strNewFile);

		std::auto_ptr<IFile> pFile(CFileFactory::CreateLoader(url));
		if (!pFile.get())
			return false;

		if (pFile->Rename(url, urlnew))
			return true;
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}
	CLog::Log(LOGERROR, "%s - Error renaming file %s", __FUNCTION__, strFile.c_str());
	return false;
}

int CFile::Stat(const CStdString& strFileName, struct __stat64* buffer)
{
	try
	{
		CURL url(strFileName);

		std::auto_ptr<IFile> pFile(CFileFactory::CreateLoader(url));
		if (!pFile.get())
			return -1;

		return pFile->Stat(url, buffer);
	}
	catch (const win32_exception &e)
	{
		e.writelog(__FUNCTION__);
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s - Unhandled exception", __FUNCTION__);
	}

	CLog::Log(LOGERROR, "%s - Error statting %s", __FUNCTION__, strFileName.c_str());
	return -1;
}

bool CFile::Cache(const CStdString& strFileName, const CStdString& strDest, IFileCallback* pCallback, void* pContext)
{
	CFile file;
	if (file.Open(strFileName, READ_TRUNCATED))
	{
		CFile newFile;
		if (CFile::Exists(strDest))
			CFile::Delete(strDest);
		if (!newFile.OpenForWrite(strDest, true))
		{
			file.Close();
			return false;
		}

		int iBufferSize = 128 * 1024;
		char *buffer = new char[iBufferSize];
		int iRead, iWrite;

		UINT64 llFileSize = file.GetLength();
		UINT64 llPos = 0;

		CStopWatch timer;
		timer.StartZero();
		float start = 0.0f;
		while (true)
		{
			iRead = file.Read(buffer, iBufferSize);
			if (iRead == 0) break;
			else if (iRead < 0)
			{
				CLog::Log(LOGERROR, "%s - Failed read from file %s", __FUNCTION__, strFileName.c_str());
				llFileSize = (UINT64)-1;
				break;
			}

			/* write data and make sure we managed to write it all */
			iWrite = 0;
			while (iWrite < iRead)
			{
				int iWrite2 = newFile.Write(buffer + iWrite, iRead - iWrite);
				if (iWrite2 <= 0)
					break;
				iWrite += iWrite2;
			}

			if (iWrite != iRead)
			{
				CLog::Log(LOGERROR, "%s - Failed write to file %s", __FUNCTION__, strDest.c_str());
				llFileSize = (UINT64)-1;
				break;
			}

			llPos += iRead;

			// calculate the current and average speeds
			float end = timer.GetElapsedSeconds();

			if (pCallback && end - start > 0.5 && end)
			{
				start = end;

				float averageSpeed = llPos / end;
				int ipercent = 0;
				if (llFileSize)
					ipercent = (int)(100 * llPos / llFileSize);

				if (!pCallback->OnFileCallback(pContext, ipercent, averageSpeed))
				{
					CLog::Log(LOGERROR, "%s - User aborted copy", __FUNCTION__);
					llFileSize = (UINT64)-1;
					break;
				}
			}
		}

		/* close both files */
		newFile.Close();
		file.Close();
		delete[] buffer;

		/* verify that we managed to completed the file */
		if (llFileSize && llPos != llFileSize)
		{
			CFile::Delete(strDest);
			return false;
		}
		return true;
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

void CFileStreamBuffer::Attach(IFile *file)
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
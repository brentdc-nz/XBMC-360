#include "utils/log.h"
#include "DVDInputStreamFile.h"
#include "utils/URIUtils.h"
#include "FileSystem/File.h"

CDVDInputStreamFile::CDVDInputStreamFile() : CDVDInputStream(DVDSTREAM_TYPE_FILE)
{
	m_pFile = NULL;
	m_eof = true;
}

CDVDInputStreamFile::~CDVDInputStreamFile()
{
	Close();
}

bool CDVDInputStreamFile::Open(const char* strFile, const std::string& content)
{
	if(!CDVDInputStream::Open(strFile, content)) return false;

	CStdString stdFile = strFile;
  
	m_pFile = new CFile();
	if(!m_pFile) return false;

	unsigned int flags = READ_TRUNCATED/* | READ_BITRATE | READ_CHUNKED*/;
  
//	if(CFileItem(strFile, false).IsInternetStream())
//   flags |= READ_CACHED;

	// Open file in binary mode
	if (!m_pFile->Open(strFile, flags))
	{
		delete m_pFile;
		m_pFile = NULL;
		return false;
	}
   
//	if(m_pFile->GetImplemenation() && (content.empty() || content == "application/octet-stream"))
//		m_content = m_pFile->GetImplemenation()->GetContent();
  
	m_eof = true;
	return true;
}

// Close file and reset everyting
void CDVDInputStreamFile::Close()
{
	if(m_pFile)
	{
		m_pFile->Close();
		delete m_pFile;
	}

	CDVDInputStream::Close();
	m_pFile = NULL;
	m_eof = true;
}

int64_t CDVDInputStreamFile::GetLength()
{
	if(m_pFile)
		return m_pFile->GetLength();

	return 0;
}

BitstreamStats CDVDInputStreamFile::GetBitstreamStats() const 
{
	if(!m_pFile)
		return m_stats; // Dummy return. defined in CDVDInputStream

	if(m_pFile->GetBitstreamStats())
		return *m_pFile->GetBitstreamStats();
	else
		return m_stats;
}

__int64 CDVDInputStreamFile::Seek(__int64 offset, int whence)
{
	if(!m_pFile) return -1;
	__int64 ret = m_pFile->Seek(offset, whence);

	// If we succeed, we are not eof anymore
	if(ret >= 0) m_eof = false;

	return ret;
}

int CDVDInputStreamFile::Read(BYTE* buf, int buf_size)
{
	if(!m_pFile) return -1;

	unsigned int ret = m_pFile->Read(buf, buf_size, NULL);

	// We currently don't support non completing reads
	if(ret <= 0) m_eof = true;

	return (int)(ret & 0xFFFFFFFF);
}

int CDVDInputStreamFile::GetBlockSize()
{
	if(m_pFile)
		return m_pFile->GetChunkSize();
	else
		return 0;
}

bool CDVDInputStreamFile::IsEOF()
{
	return !m_pFile || m_eof;
}
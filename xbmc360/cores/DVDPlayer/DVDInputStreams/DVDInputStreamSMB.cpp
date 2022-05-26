#include "DVDInputStreamSMB.h"

using namespace XFILE;

CDVDInputStreamSMB::CDVDInputStreamSMB() : CDVDInputStream(DVDSTREAM_TYPE_FFMPEG)
{
	m_pFile = NULL;
	m_eof = true;
}

CDVDInputStreamSMB::~CDVDInputStreamSMB()
{
	Close();  
}

bool CDVDInputStreamSMB::IsEOF()
{
	return false;
}

bool CDVDInputStreamSMB::Open(const char* strFile, const std::string& content)
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
void CDVDInputStreamSMB::Close()
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

int CDVDInputStreamSMB::Read(BYTE* buf, int buf_size)
{
	if(!m_pFile) return -1;

	unsigned int ret = m_pFile->Read(buf, buf_size, NULL);

	// We currently don't support non completing reads
	if(ret <= 0) m_eof = true;

	return (int)(ret & 0xFFFFFFFF);
}

__int64 CDVDInputStreamSMB::GetLength()
{
	if(m_pFile)
		return m_pFile->GetLength();

	return 0;
}

__int64 CDVDInputStreamSMB::Seek(__int64 offset, int whence)
{
	if(!m_pFile) return -1;
	__int64 ret = m_pFile->Seek(offset, whence);

	// If we succeed, we are not eof anymore
	if(ret >= 0) m_eof = false;

	return ret;
}


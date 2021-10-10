#include "DVDInputStreamFile.h"

using namespace XFILE;

CDVDInputStreamFile::CDVDInputStreamFile() : CDVDInputStream(DVDSTREAM_TYPE_FILE)
{
  m_pFile = NULL;
}

CDVDInputStreamFile::~CDVDInputStreamFile()
{
	Close();
}

bool CDVDInputStreamFile::IsEOF()
{
	if(m_pFile)
	{
		__int64 size = m_pFile->GetLength();
		
		if( size > 0 && m_pFile->GetPosition() >= size )
			return true;

		return false;
	}

	return true;
}

bool CDVDInputStreamFile::Open(const char* strFile/*, const std::string& content*/)
{
	if(!CDVDInputStream::Open(strFile/*, content*/)) return false;

	m_pFile = new CFile();
	if (!m_pFile) return false;

	// Open file in binary mode
	if (!m_pFile->Open(strFile, true))
	{
		delete m_pFile;
		m_pFile = NULL;
		return false;
	}

	return true;
}

// Close file and reset everyting
void CDVDInputStreamFile::Close()
{
	if (m_pFile)
	{
		m_pFile->Close();
		delete m_pFile;
	}

	CDVDInputStream::Close();
	m_pFile = NULL;
}
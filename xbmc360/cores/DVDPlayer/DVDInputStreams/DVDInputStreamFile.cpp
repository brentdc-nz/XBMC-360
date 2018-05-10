#include "DVDInputStreamFile.h"

CDVDInputStreamFile::CDVDInputStreamFile() : CDVDInputStream()
{
	m_streamType = DVDSTREAM_TYPE_FILE;
}

CDVDInputStreamFile::~CDVDInputStreamFile()
{
	Close();
}

bool CDVDInputStreamFile::Open(const char* strFile)
{
	if (!CDVDInputStream::Open(strFile)) return false;

	return true;
}

// Close file and reset everyting
void CDVDInputStreamFile::Close()
{
	CDVDInputStream::Close(); 
}
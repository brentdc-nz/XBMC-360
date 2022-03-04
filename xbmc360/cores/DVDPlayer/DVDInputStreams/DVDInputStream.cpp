#include "DVDInputStream.h"

CDVDInputStream::CDVDInputStream(DVDStreamType streamType)
{
	m_streamType = streamType;
}

CDVDInputStream::~CDVDInputStream()
{
}

bool CDVDInputStream::Open(const char* strFile, const std::string &content)
{
	m_strFileName = strFile;
	m_content = content;
	return true;
}

void CDVDInputStream::Close()
{
	m_strFileName = "";
	m_item.Reset();
}

void CDVDInputStream::SetFileItem(const CFileItem& item)
{
	m_item = item;
}
	
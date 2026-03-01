#include "DVDInputStream.h"
#include "URL.h"

CDVDInputStream::CDVDInputStream(DVDStreamType streamType)
{
	m_streamType = streamType;
}

CDVDInputStream::~CDVDInputStream()
{
}

bool CDVDInputStream::Open(const char* strFile, const std::string &content)
{
	CURL url = CURL(strFile);

	// Get rid of any |option parameters which might have sneaked in here
	// those are only handled by our curl impl.
//	url.SetProtocolOptions("");
	m_strFileName = url.Get();

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
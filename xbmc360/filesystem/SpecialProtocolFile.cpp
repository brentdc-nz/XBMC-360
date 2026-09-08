#include "SpecialProtocolFile.h"
#include "SpecialProtocol.h"

using namespace XFILE;

CSpecialProtocolFile::CSpecialProtocolFile(void)
{
}

CSpecialProtocolFile::~CSpecialProtocolFile(void)
{
	Close();
}

bool CSpecialProtocolFile::Open(const CURL& url)
{
	CStdString strFileName = CSpecialProtocol::TranslatePath(url);
	return m_file.Open(strFileName);
}

bool CSpecialProtocolFile::OpenForWrite(const CURL& url, bool bOverWrite /*=false*/)
{
	CStdString strFileName = CSpecialProtocol::TranslatePath(url);
	return m_file.OpenForWrite(strFileName, bOverWrite);
}

bool CSpecialProtocolFile::Delete(const CURL& url)
{
	CStdString strFileName = CSpecialProtocol::TranslatePath(url);
	return CFile::Delete(strFileName);
}

bool CSpecialProtocolFile::Exists(const CURL& url)
{
	CStdString strFileName = CSpecialProtocol::TranslatePath(url);
	return CFile::Exists(strFileName);
}

int CSpecialProtocolFile::Stat(const CURL& url, struct __stat64* buffer)
{
	CStdString strFileName = CSpecialProtocol::TranslatePath(url);
	return CFile::Stat(strFileName, buffer);
}

bool CSpecialProtocolFile::Rename(const CURL& url, const CURL& urlnew)
{
	CStdString strFileName = CSpecialProtocol::TranslatePath(url);
	CStdString strFileName2 = CSpecialProtocol::TranslatePath(urlnew);
	return CFile::Rename(strFileName, strFileName2);
}

unsigned int CSpecialProtocolFile::Read(void* lpBuf, int64_t uiBufSize)
{
	return m_file.Read(lpBuf, uiBufSize);
}

int CSpecialProtocolFile::Write(const void* lpBuf, int64_t uiBufSize)
{
	return m_file.Write(lpBuf, uiBufSize);
}

int64_t CSpecialProtocolFile::Seek(int64_t iFilePosition, int iWhence)
{
	return m_file.Seek(iFilePosition, iWhence);
}

void CSpecialProtocolFile::Close()
{
	m_file.Close();
}

int64_t CSpecialProtocolFile::GetPosition()
{
	return m_file.GetPosition();
}

int64_t CSpecialProtocolFile::GetLength()
{
	return m_file.GetLength();
}

#include "URL.h"

CURL::CURL(const CStdString& strURL1)
{
	Parse(strURL1);
}

CURL::CURL()
{
}

CURL::~CURL()
{
}

void CURL::Reset()
{
  m_strFileName.clear();
  m_strProtocol.clear();
}

void CURL::Parse(const CStdString& strURL)
{
	Reset();

	// strURL can be one of the following:
	// Format 1: protocol://[username:password]@hostname[:port]/directoryandfile
	// Format 2: protocol://file
	// Format 3: drive:directoryandfile

	// First need 2 check if this is a protocol or just a normal drive & path
	if(!strURL.size()) return;
	if(strURL.Equals("?", true)) return;

	// Decode the protocol
	int iPos = strURL.Find("://");
	SetProtocol(strURL.Left(iPos));
	iPos += 3;

	SetFileName(strURL/*strURL.Mid(iPos)*/);
}

void CURL::SetProtocol(const CStdString& strProtocol)
{
	m_strProtocol = strProtocol;
	m_strProtocol.ToLower();
}

const CStdString& CURL::GetProtocol() const
{
	return m_strProtocol;
}

void CURL::SetFileName(const CStdString& strFileName)
{
	m_strFileName = strFileName;
	
	// TODO: Add file type detection code
}

const CStdString& CURL::GetFileName() const
{
	return m_strFileName;
}
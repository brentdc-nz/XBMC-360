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

void CURL::SetFileName(const CStdString& strFileName)
{
	m_strFileName = strFileName;

	int slash = m_strFileName.find_last_of(GetDirectorySeparator());
	int period = m_strFileName.find_last_of('.');
	
	if(period != -1 && (slash == -1 || period > slash))
		m_strFileType = m_strFileName.substr(period+1);
	else
		m_strFileType = "";

	m_strFileType.Normalize();
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

void CURL::GetURL(CStdString& strURL) const
{
	unsigned int sizeneed = m_strProtocol.length()
//							+ m_strDomain.length()
//							+ m_strUserName.length()
//							+ m_strPassword.length()
//							+ m_strHostName.length()
							+ m_strFileName.length() 
//							+ m_strOptions.length();
							+ 10;

	if( strURL.capacity() < sizeneed)
		strURL.reserve(sizeneed);

	if (m_strProtocol == "")
	{
		strURL = m_strFileName;
		return;
	}

	GetURLWithoutFilename(strURL);
	strURL += m_strFileName;

//	if(m_strOptions.length() > 0)
//		strURL += m_strOptions;
}

const CStdString& CURL::GetFileName() const
{
	return m_strFileName;
}

const char CURL::GetDirectorySeparator() const
{
	if(IsLocal()) 
		return '\\';
	else
		return '/';
}

void CURL::GetURLWithoutUserDetails(CStdString& strURL) const
{
	unsigned int sizeneed = m_strProtocol.length()
//						+ m_strDomain.length()
//						+ m_strHostName.length()
						+ m_strFileName.length() 
//						+ m_strOptions.length();
						+ 10;

	if(strURL.capacity() < sizeneed)
		strURL.reserve(sizeneed);

	if(m_strProtocol == "")
	{
		strURL = m_strFileName;
		return;
	}

	strURL = m_strProtocol;
	strURL += "://";
/*
	if(m_strHostName != "")
	{
		strURL += m_strHostName;
		if(HasPort())
		{
			CStdString strPort;
			strPort.Format("%i", m_iPort);
			strURL += ":";
			strURL += strPort;
		}
		strURL += "/";
	}
*/	strURL += m_strFileName;

//	if(m_strOptions.length() > 0)
//		strURL += m_strOptions;
}

void CURL::GetURLWithoutFilename(CStdString& strURL) const
{
	unsigned int sizeneed = m_strProtocol.length()
//						+ m_strDomain.length()
//						+ m_strUserName.length()
 //                       + m_strPassword.length()
 //                       + m_strHostName.length()
                        + 10;

  if( strURL.capacity() < sizeneed )
    strURL.reserve(sizeneed);


  if (m_strProtocol == "")
  {
    strURL = m_strFileName.substr(0, 2); // only copy 'e:'
    return ;
  }
/*
  strURL = m_strProtocol;
  strURL += "://";

  if (m_strDomain != "")
  {
    strURL += m_strDomain;
    strURL += ";";
  }
  if (m_strUserName != "" && m_strPassword != "")
  {
    strURL += URLEncodeInline(m_strUserName);
    strURL += ":";
    strURL += URLEncodeInline(m_strPassword);
    strURL += "@";
  }
  else if (m_strUserName != "")
  {
    strURL += URLEncodeInline(m_strUserName);
    strURL += ":";
    strURL += "@";
  }
  else if (m_strDomain != "")
    strURL += "@";

  if (m_strHostName != "")
  {
    if( m_strProtocol.Equals("rar") || m_strProtocol.Equals("zip") )
      strURL += URLEncodeInline(m_strHostName);
    else
      strURL += m_strHostName;
    if (HasPort())
    {
      CStdString strPort;
      strPort.Format("%i", m_iPort);
      strURL += ":";
      strURL += strPort;
    }
    strURL += "/";
  }*/
}

bool CURL::IsLocal() const
{
	return m_strProtocol.IsEmpty();
}
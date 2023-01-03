#include "url.h"
#include "utils\stdafx.h"
#include "utils\Util.h"
#include "utils\Log.h"
#include "FileItem.h"

CStdString URLEncodeInline(const CStdString& strData)
{
	CStdString buffer = strData;
	CUtil::URLEncode(buffer);
	return buffer;
}

CURL::CURL(const CStdString& strURL)
{
	m_strHostName = "";
	m_strDomain = "";
	m_strUserName = "";
	m_strPassword = "";
	m_strShareName="";
	m_strFileName = "";
	m_strProtocol = "";
	m_strFileType = "";
	m_iPort = 0;

	// strURL can be one of the following:
	// Format 1: protocol://[username:password]@hostname[:port]/directoryandfile
	// Format 2: protocol://file
	// Format 3: drive:directoryandfile
	//
	// First need 2 check if this is a protocol or just a normal drive & path

	if(!strURL.size()) return ;
	if(strURL.Equals("?", true)) return;
	if(strURL[1] == ':' || strURL[4] == ':')
	{
		// Form is drive:directoryandfile

		// Set filename and update extension
		SetFileName(strURL);
		return;
	}

	// Form is format 1 or 2
	// Format 1: protocol://[domain;][username:password]@hostname[:port]/directoryandfile
	// Format 2: protocol://file

	// Decode protocol
	int iPos = strURL.Find("://");
	if(iPos < 0)
	{
		// Check for misconstructed protocols
		iPos = strURL.Find(":");
		if(iPos == strURL.GetLength() - 1)
		{
			m_strProtocol = strURL.Left(iPos);
			iPos += 1;
		}
		else
		{
			CLog::Log(LOGDEBUG, __FUNCTION__" - Url has no protocol %s, empty CURL created", strURL.c_str());
			return;
		}
	}
	else
	{
		m_strProtocol = strURL.Left(iPos);
		iPos += 3;
	}

	// Virtual protocols
	// why not handle all format 2 (protocol://file) style urls here?
	// ones that come to mind are iso9660, cdda, musicdb, etc.
	// they are all local protocols and have no server part, port number, special options, etc.
	// this removes the need for special handling below.
	if( m_strProtocol.Equals("stack") ||
		m_strProtocol.Equals("virtualpath") ||
		m_strProtocol.Equals("multipath") ||
		m_strProtocol.Equals("filereader") )
	{
		m_strFileName = strURL.Mid(iPos);
		return;
	}

	// Check for username/password - should occur before first /
	if(iPos == -1) iPos = 0;

	// For protocols supporting options, chop that part off here
	int iEnd = strURL.length();
	if( m_strProtocol.Equals("http")
		|| m_strProtocol.Equals("https")
		|| m_strProtocol.Equals("ftp")
		|| m_strProtocol.Equals("ftpx")
		|| m_strProtocol.Equals("shout")
		|| m_strProtocol.Equals("tuxbox")
		|| m_strProtocol.Equals("daap") )
	{
		int iOptions = strURL.find_first_of("?;#", iPos);
		if(iOptions >= 0)
		{
			// We keep the initial char as it can be any of the above
			m_strOptions = strURL.substr(iOptions);
			iEnd = iOptions;
		}
	}

	int iSlash = strURL.Find("/", iPos);
	if(iSlash >= iEnd)
		iSlash = -1; // Was an invalid slash as it was contained in options

	if(!m_strProtocol.Equals("iso9660"))
	{
		int iAlphaSign = strURL.Find("@", iPos);
		if(iAlphaSign >= 0 && iAlphaSign < iEnd && (iAlphaSign < iSlash || iSlash < 0))
		{
			// username/password found
			CStdString strUserNamePassword = strURL.Mid(iPos, iAlphaSign - iPos);

			// First extract domain, if protocol is smb
			if(m_strProtocol.Equals("smb"))
			{
				int iSemiColon = strUserNamePassword.Find(";");

				if(iSemiColon >= 0)
				{
					m_strDomain = strUserNamePassword.Left(iSemiColon);
					strUserNamePassword.Delete(0, iSemiColon + 1);
				}
			}

			// username:password
			int iColon = strUserNamePassword.Find(":");
			if(iColon >= 0)
			{
				m_strUserName = strUserNamePassword.Left(iColon);
				iColon++;
				m_strPassword = strUserNamePassword.Right(strUserNamePassword.size() - iColon);
			}
			// username
			else
				m_strUserName = strUserNamePassword;

			iPos = iAlphaSign + 1;
			iSlash = strURL.Find("/", iAlphaSign);

			if(iSlash >= iEnd)
			iSlash = -1;
		}
	}

	// Detect hostname:port/
	if(iSlash < 0)
	{
		CStdString strHostNameAndPort = strURL.Mid(iPos, iEnd - iPos);
		int iColon = strHostNameAndPort.Find(":");
		if(iColon >= 0)
		{
			m_strHostName = strHostNameAndPort.Left(iColon);
			iColon++;
			CStdString strPort = strHostNameAndPort.Right(strHostNameAndPort.size() - iColon);
			m_iPort = atoi(strPort.c_str());
		}
		else
		  m_strHostName = strHostNameAndPort;
	}
	else
	{
		CStdString strHostNameAndPort = strURL.Mid(iPos, iSlash - iPos);
		int iColon = strHostNameAndPort.Find(":");
		if(iColon >= 0)
		{
			m_strHostName = strHostNameAndPort.Left(iColon);
			iColon++;
			CStdString strPort = strHostNameAndPort.Right(strHostNameAndPort.size() - iColon);
			m_iPort = atoi(strPort.c_str());
		}
		else
			m_strHostName = strHostNameAndPort;

		iPos = iSlash + 1;
		if(iEnd > iPos)
		{
			m_strFileName = strURL.Mid(iPos, iEnd - iPos);
			iSlash = m_strFileName.Find("/");
#if 0
			if(iSlash < 0)
				m_strShareName = m_strFileName;
			else
				m_strShareName = m_strFileName.Left(iSlash);
#else
			m_strShareName = m_strFileName;
#endif 
		}
	}

	// iso9960 doesnt have an hostname
	if (m_strProtocol.CompareNoCase("iso9660") == 0
		|| m_strProtocol.CompareNoCase("musicdb") == 0
		|| m_strProtocol.CompareNoCase("videodb") == 0
		|| m_strProtocol.CompareNoCase("lastfm") == 0
		|| m_strProtocol.Left(3).CompareNoCase("mem") == 0)
	{
		if(m_strHostName != "" && m_strFileName != "")
		{
			CStdString strFileName = m_strFileName;
			m_strFileName.Format("%s/%s", m_strHostName.c_str(), strFileName.c_str());
			m_strHostName = "";
		}
		else
		{
			if(!m_strHostName.IsEmpty() && strURL[iEnd-1]=='/')
				m_strFileName=m_strHostName + "/";
			else
				m_strFileName = m_strHostName;

			m_strHostName = "";
		}
	}

	m_strFileName.Replace("\\", "/");

	// Update extension
	SetFileName(m_strFileName);

	// Decode urlencoding on this stuff
	if(m_strProtocol.Equals("rar") || m_strProtocol.Equals("zip") || m_strProtocol.Equals("musicsearch"))
		CUtil::UrlDecode(m_strHostName);

	CUtil::UrlDecode(m_strUserName);
	CUtil::UrlDecode(m_strPassword);
}

CURL::CURL(const CURL &url)
{
	*this = url;
}

CURL::CURL()
{
	m_iPort = 0;
}

CURL::~CURL()
{
}

CURL& CURL::operator= (const CURL& source)
{
	m_iPort        = source.m_iPort;
	m_strHostName  = source.m_strHostName;
	m_strDomain    = source.m_strDomain;
	m_strShareName = source.m_strShareName;
	m_strUserName  = source.m_strUserName;
	m_strPassword  = source.m_strPassword;
	m_strFileName  = source.m_strFileName;
	m_strProtocol  = source.m_strProtocol;
	m_strFileType  = source.m_strFileType;
	m_strOptions   = source.m_strOptions;

	return *this;
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

void CURL::SetHostName(const CStdString& strHostName)
{
	m_strHostName = strHostName;
}

void CURL::SetUserName(const CStdString& strUserName)
{
	m_strUserName = strUserName;
}

void CURL::SetPassword(const CStdString& strPassword)
{
	m_strPassword = strPassword;
}

void CURL::SetProtocol(const CStdString& strProtocol)
{
	m_strProtocol = strProtocol;
}

void CURL::SetOptions(const CStdString& strOptions)
{
	m_strOptions.Empty();
	if(strOptions.length() > 0)
	if(strOptions[0] == '?' || strOptions[0] == '#' || strOptions[0] == ';' || strOptions.Find("xml") >=0)
	{
      m_strOptions = strOptions;
    }
    else
      CLog::Log(LOGWARNING, __FUNCTION__" - Invalid options specified for url %s", strOptions.c_str());
}

void CURL::SetPort(int port)
{
	m_iPort = port;
}


bool CURL::HasPort() const
{
	return (m_iPort != 0);
}

int CURL::GetPort() const
{
	return m_iPort;
}

const CStdString& CURL::GetHostName() const
{
	return m_strHostName;
}

const CStdString&  CURL::GetShareName() const
{
	return m_strShareName;
}

const CStdString& CURL::GetDomain() const
{
	return m_strDomain;
}

const CStdString& CURL::GetUserName() const
{
	return m_strUserName;
}

const CStdString& CURL::GetPassWord() const
{
	return m_strPassword;
}

const CStdString& CURL::GetFileName() const
{
	return m_strFileName;
}

const CStdString& CURL::GetProtocol() const
{
	return m_strProtocol;
}

const CStdString& CURL::GetFileType() const
{
	return m_strFileType;
}

const CStdString& CURL::GetOptions() const
{
	return m_strOptions;
}

const CStdString CURL::GetFileNameWithoutPath() const
{
	return CUtil::GetFileName(m_strFileName);
}

const char CURL::GetDirectorySeparator() const
{
	if(IsLocal())
		return '\\';
	else
		return '/';
}

CStdString CURL::Get() const
{
	unsigned int sizeneed = m_strProtocol.length()
							+ m_strDomain.length()
							+ m_strUserName.length()
							+ m_strPassword.length()
							+ m_strHostName.length()
							+ m_strFileName.length()
							+ m_strOptions.length()
							+ m_strProtocolOptions.length()
							+ 10;

	if(m_strProtocol == "")
		return m_strFileName;

	CStdString strURL;
	strURL.reserve(sizeneed);

	strURL = GetWithoutFilename();
	strURL += m_strFileName;

	if(m_strOptions.length() > 0 )
		strURL += m_strOptions;

	if(m_strProtocolOptions.length() > 0)
		strURL += "|"+m_strProtocolOptions;

	return strURL;
}

CStdString CURL::GetWithoutUserDetails() const
{
	CStdString strURL;

	if (m_strProtocol.Equals("stack")) // TODO
	{
		CFileItemList items;
		CStdString strURL2;
		strURL2 = Get();
/*		XFILE::CStackDirectory dir; // TODO
		dir.GetDirectory(strURL2,items);
		vector<CStdString> newItems;
		
		for (int i = 0; i < items.Size(); ++i)
		{
			CURL url(items[i]->GetPath());
			items[i]->SetPath(url.GetWithoutUserDetails());
			newItems.push_back(items[i]->GetPath());
		}
		
		dir.ConstructStackPath(newItems,strURL);
*/		return strURL;
	}

	unsigned int sizeneed = m_strProtocol.length()
							+ m_strDomain.length()
							+ m_strHostName.length()
							+ m_strFileName.length()
							+ m_strOptions.length()
							+ m_strProtocolOptions.length()
							+ 10;

	strURL.reserve(sizeneed);

	if (m_strProtocol == "")
		return m_strFileName;

	strURL = m_strProtocol;
	strURL += "://";

	if (m_strHostName != "")
	{
		if (m_strProtocol.Equals("rar") || m_strProtocol.Equals("zip"))
			strURL += CURL(m_strHostName).GetWithoutUserDetails();
		else
			strURL += m_strHostName;

		if ( HasPort() )
		{
			CStdString strPort;
			strPort.Format("%i", m_iPort);
			strURL += ":";
			strURL += strPort;
		}
		strURL += "/";
	}

	strURL += m_strFileName;

	if( m_strOptions.length() > 0 )
		strURL += m_strOptions;
	
	if( m_strProtocolOptions.length() > 0 )
		strURL += "|"+m_strProtocolOptions;

	return strURL;
}

void CURL::GetURL(CStdString& strURL) const
{
	unsigned int sizeneed = m_strProtocol.length()
							+ m_strDomain.length()
							+ m_strUserName.length()
							+ m_strPassword.length()
							+ m_strHostName.length()
							+ m_strFileName.length()
							+ m_strOptions.length();
							+ 10;

	if(strURL.capacity() < sizeneed)
		strURL.reserve(sizeneed);

	if(m_strProtocol == "")
	{
		strURL = m_strFileName;
		return;
	}

	GetURLWithoutFilename(strURL);
	strURL += m_strFileName;

	if(m_strOptions.length() > 0)
		strURL += m_strOptions;
}

void CURL::GetURLWithoutUserDetails(CStdString& strURL) const
{
	unsigned int sizeneed = m_strProtocol.length()
							+ m_strDomain.length()
							+ m_strHostName.length()
							+ m_strFileName.length()
							+ m_strOptions.length();
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

	if(m_strHostName != "")
	{
		if(m_strProtocol.Equals("rar") || m_strProtocol.Equals("zip"))
		{
			CURL url2(m_strHostName);
			CStdString strHost;
			url2.GetURLWithoutUserDetails(strHost);
			strURL += strHost;
		}
		else
			strURL += m_strHostName;

		if( HasPort())
		{
			CStdString strPort;
			strPort.Format("%i", m_iPort);
			strURL += ":";
			strURL += strPort;
		}
		strURL += "/";
	}
	strURL += m_strFileName;

	if(m_strOptions.length() > 0)
		strURL += m_strOptions;
}

CStdString CURL::GetWithoutFilename() const
{
	if(m_strProtocol == "")
		return "";

	unsigned int sizeneed = m_strProtocol.length()
						+ m_strDomain.length()
						+ m_strUserName.length()
						+ m_strPassword.length()
						+ m_strHostName.length()
						+ 10;

	CStdString strURL;
	strURL.reserve(sizeneed);

	strURL = m_strProtocol;
	strURL += "://";

	if(m_strDomain != "")
	{
		strURL += m_strDomain;
		strURL += ";";
	}
	else if(m_strUserName != "")
	{
		strURL += URLEncodeInline(m_strUserName);
		if(m_strPassword != "")
		{
			strURL += ":";
			strURL += URLEncodeInline(m_strPassword);
		}
		strURL += "@";
	}
	else if (m_strDomain != "")
		strURL += "@";

	if(m_strHostName != "")
	{
		if(m_strProtocol.Equals("rar") || m_strProtocol.Equals("zip") || m_strProtocol.Equals("musicsearch"))
			strURL += URLEncodeInline(m_strHostName);
		else
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
	return strURL;
}

void CURL::GetURLWithoutFilename(CStdString& strURL) const
{
	unsigned int sizeneed = m_strProtocol.length()
							+ m_strDomain.length()
							+ m_strUserName.length()
							+ m_strPassword.length()
							+ m_strHostName.length()
							+ 10;

	if(strURL.capacity() < sizeneed)
		strURL.reserve(sizeneed);

	if(m_strProtocol == "")
	{
		strURL = m_strFileName.substr(0, 2); // only copy 'e:'
		return;
	}

	strURL = m_strProtocol;
	strURL += "://";

	if(m_strDomain != "")
	{
		strURL += m_strDomain;
		strURL += ";";
	}

	if(m_strUserName != "" && m_strPassword != "")
	{
		strURL += URLEncodeInline(m_strUserName);
		strURL += ":";
		strURL += URLEncodeInline(m_strPassword);
		strURL += "@";
	}
	else if(m_strUserName != "")
	{
		strURL += URLEncodeInline(m_strUserName);
		strURL += ":";
		strURL += "@";
	}
	else if(m_strDomain != "")
		strURL += "@";

	if(m_strHostName != "")
	{
		if(m_strProtocol.Equals("rar") || m_strProtocol.Equals("zip"))
			strURL += URLEncodeInline(m_strHostName);
		else
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
}

bool CURL::IsLocal() const
{
	return m_strProtocol.IsEmpty();
}

bool CURL::IsFullPath(const CStdString &url)
{
	if(url.size() && url[0] == '/') return true; // - /foo/bar.ext
	if(url.Find("://") >= 0) return true; // - foo://bar.ext
	if(url.size() > 1 && url[1] == ':') return true; // - c:\\foo\\bar\\bar.ext
	
	return false;
}

// Modified to be more accomodating - If a non hex value follows a % take the characters directly and don't raise an error.
// However % characters should really be escaped like any other non safe character (www.rfc-editor.org/rfc/rfc1738.txt)
void CURL::Decode(CStdString& strURLData)
{
	CStdString strResult;

	// result will always be less than source
	strResult.reserve( strURLData.length() );

	for (unsigned int i = 0; i < strURLData.size(); ++i)
	{
		int kar = (unsigned char)strURLData[i];

		if (kar == '+') strResult += ' ';
		else if (kar == '%')
		{
			if (i < strURLData.size() - 2)
			{
				CStdString strTmp;
				strTmp.assign(strURLData.substr(i + 1, 2));
				int dec_num=-1;
				sscanf(strTmp,"%x",(unsigned int *)&dec_num);
				
				if (dec_num<0 || dec_num>255)
					strResult += kar;
				else
				{
					strResult += (char)dec_num;
					i += 2;
				}
			}
			else
				strResult += kar;
		}
		else strResult += kar;
	}
	strURLData = strResult;
}

void CURL::Encode(CStdString& strURLData)
{
	CStdString strResult;

	// Wonder what a good value is here is, depends on how often it occurs
	strResult.reserve( strURLData.length() * 2 );

	for (int i = 0; i < (int)strURLData.size(); ++i)
	{
		int kar = (unsigned char)strURLData[i];

		//if (kar == ' ') strResult += '+';
		if (isalnum(kar)) strResult
			+= kar;
		else
		{
			CStdString strTmp;
			strTmp.Format("%%%02.2x", kar);
			strResult += strTmp;
		}
	}
	strURLData = strResult;
}
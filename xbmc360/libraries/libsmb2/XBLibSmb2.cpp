#include "XBLibSmb2.h"
#include "utils\Log.h"
#include <fcntl.h>

CXBLibSMB2::CXBLibSMB2()
{
	m_pLibSMB2Context = NULL;
	m_pLibSMB2H = NULL;
	m_pLibSMB2Url = NULL;
	m_FileSize = 0;
	m_pLibSMB2FH = NULL;
}

CXBLibSMB2::~CXBLibSMB2()
{
}

bool CXBLibSMB2::Init()
{
	Close(); // Hack: Why are these not been closed properly by the player?!

	if(!m_pLibSMB2Context)
	{
		m_pLibSMB2Context = smb2_init_context();

		if(m_pLibSMB2Context == NULL)
		{
			CLog::Log(LOGERROR, "Failed to init context\n");
			return false;
		}
	}

	return true;
}

bool CXBLibSMB2::OpenDir(const CURL& url)
{
	if(!m_pLibSMB2Context)
		return false;

	smb2_set_user(m_pLibSMB2Context, url.GetUserName());		
	smb2_set_password(m_pLibSMB2Context, url.GetPassWord());
	smb2_set_domain(m_pLibSMB2Context, "workgroup");
		
	m_pLibSMB2Url = smb2_parse_url(m_pLibSMB2Context, "smb://"+url.GetHostName()+":445/"+url.GetShareName());

	if(!m_pLibSMB2Url)
	{
		CLog::Log(LOGERROR, "Failed to parse url: %s\n", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	smb2_set_security_mode(m_pLibSMB2Context, SMB2_NEGOTIATE_SIGNING_ENABLED);

	// Connect to the share 
	if(smb2_connect_share(m_pLibSMB2Context, m_pLibSMB2Url->server, m_pLibSMB2Url->share, m_pLibSMB2Url->user) != 0)
	{
		CLog::Log(LOGERROR, "smb2_connect_share failed. %s", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	m_pLibSMB2H = smb2_opendir(m_pLibSMB2Context, m_pLibSMB2Url->path);

	if(!m_pLibSMB2H)
		return false;
	
	return true;
}

smb2dirent* CXBLibSMB2::ReadDir()
{
	return smb2_readdir(m_pLibSMB2Context, m_pLibSMB2H);
}

bool CXBLibSMB2::OpenFile(const CURL& url)
{
	if(!m_pLibSMB2Context)
		return false;

	smb2_set_user(m_pLibSMB2Context, url.GetUserName());		
	smb2_set_password(m_pLibSMB2Context, url.GetPassWord());
	smb2_set_domain(m_pLibSMB2Context, "workgroup");
		
	m_pLibSMB2Url = smb2_parse_url(m_pLibSMB2Context, "smb://"+url.GetHostName()+":445/"+url.GetShareName());

	if(!m_pLibSMB2Url)
	{
		CLog::Log(LOGERROR, "Failed to parse url: %s\n", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	smb2_set_security_mode(m_pLibSMB2Context, SMB2_NEGOTIATE_SIGNING_ENABLED);

	// Connect to the share 
	if(smb2_connect_share(m_pLibSMB2Context, m_pLibSMB2Url->server, m_pLibSMB2Url->share, m_pLibSMB2Url->user) != 0)
	{
		CLog::Log(LOGERROR, "smb2_connect_share failed. %s", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	m_pLibSMB2FH = smb2_open(m_pLibSMB2Context, m_pLibSMB2Url->path, O_RDONLY | O_BINARY);

	if(m_pLibSMB2FH == NULL)
		return false;

	m_FileSize = smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, 0, SEEK_END, NULL);

	// We've opened the file!
	return true;
}

__int64 CXBLibSMB2::Seek(__int64 iFilePosition, int iWhence)
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return -1;

	INT64 iPos = smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, iFilePosition, iWhence, NULL);

	if(iPos < 0)
		return -1;

	return(__int64)iPos;
}

unsigned int CXBLibSMB2::Read(void *lpBuf, __int64 uiBufSize)
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return 0;

	int bytesRead = smb2_read(m_pLibSMB2Context, m_pLibSMB2FH,static_cast<uint8_t *>(lpBuf)/* (unsigned __int8*)lpBuf*/, (unsigned int)uiBufSize);

	if(bytesRead < 0)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - smbc_read returned error %i", errno);
		return 0;
	}

	return (unsigned int)bytesRead;
}

__int64 CXBLibSMB2::GetLength()
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return 0;

	return m_FileSize;
}

__int64 CXBLibSMB2::GetPosition()
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return 0;

	__int64 iPos = smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, 0, SEEK_CUR, NULL);
	
	if(iPos < 0)
		return 0;
	
	return iPos;
}

void CXBLibSMB2::Close()
{
	if(m_pLibSMB2H)
	{
		smb2_closedir(m_pLibSMB2Context, m_pLibSMB2H);
		m_pLibSMB2Context = NULL;
	}

	if(m_pLibSMB2FH)
	{
		smb2_close(m_pLibSMB2Context, m_pLibSMB2FH);
		m_pLibSMB2FH = NULL;
	}

	if(m_pLibSMB2Url)
	{
		smb2_destroy_url(m_pLibSMB2Url);
		m_pLibSMB2Url = NULL;
	}

	if(m_pLibSMB2Context)
	{
		smb2_disconnect_share(m_pLibSMB2Context);
		m_pLibSMB2Context = NULL;
	}

	m_FileSize = 0;
}
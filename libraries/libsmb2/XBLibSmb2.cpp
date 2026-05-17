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
	Close(); // Ensure cleanup on destruction
}

bool CXBLibSMB2::Init()
{
	// Close any open file/directory handles, but keep connection alive
	CloseHandle();

	// Only create context if we don't have one
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

bool CXBLibSMB2::IsConnectedToShare(const char* server, const char* share)
{
	if(!m_pLibSMB2Context || m_ConnectedServer.IsEmpty() || m_ConnectedShare.IsEmpty())
		return false;
	
	return (m_ConnectedServer.CompareNoCase(server) == 0 && 
	        m_ConnectedShare.CompareNoCase(share) == 0);
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

	// Only connect if not already connected to this share
	if(!IsConnectedToShare(m_pLibSMB2Url->server, m_pLibSMB2Url->share))
	{
		// If we were previously connected to a different share, tear down
		// the old connection first so the context is in a clean state
		if(!m_ConnectedServer.IsEmpty())
		{
			smb2_disconnect_share(m_pLibSMB2Context);
			m_ConnectedServer.Empty();
			m_ConnectedShare.Empty();
		}

		if(smb2_connect_share(m_pLibSMB2Context, m_pLibSMB2Url->server, m_pLibSMB2Url->share, m_pLibSMB2Url->user) != 0)
		{
			CLog::Log(LOGERROR, "smb2_connect_share failed. %s", smb2_get_error(m_pLibSMB2Context));
			return false;
		}
		m_ConnectedServer = m_pLibSMB2Url->server;
		m_ConnectedShare = m_pLibSMB2Url->share;
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

	// Only connect if not already connected to this share
	if(!IsConnectedToShare(m_pLibSMB2Url->server, m_pLibSMB2Url->share))
	{
		// If we were previously connected to a different share, tear down
		// the old connection first so the context is in a clean state
		if(!m_ConnectedServer.IsEmpty())
		{
			smb2_disconnect_share(m_pLibSMB2Context);
			m_ConnectedServer.Empty();
			m_ConnectedShare.Empty();
		}

		if(smb2_connect_share(m_pLibSMB2Context, m_pLibSMB2Url->server, m_pLibSMB2Url->share, m_pLibSMB2Url->user) != 0)
		{
			CLog::Log(LOGERROR, "smb2_connect_share failed. %s", smb2_get_error(m_pLibSMB2Context));
			return false;
		}
		m_ConnectedServer = m_pLibSMB2Url->server;
		m_ConnectedShare = m_pLibSMB2Url->share;
	}

	m_pLibSMB2FH = smb2_open(m_pLibSMB2Context, m_pLibSMB2Url->path, O_RDONLY | O_BINARY);

	if(m_pLibSMB2FH == NULL)
		return false;

	m_FileSize = smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, 0, SEEK_END, NULL);

	// Seek back to the beginning so reads start from position 0
	smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, 0, SEEK_SET, NULL);

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

void CXBLibSMB2::CloseHandle()
{
	// Close directory handle
	if(m_pLibSMB2H)
	{
		smb2_closedir(m_pLibSMB2Context, m_pLibSMB2H);
		m_pLibSMB2H = NULL;
	}

	// Close file handle
	if(m_pLibSMB2FH)
	{
		smb2_close(m_pLibSMB2Context, m_pLibSMB2FH);
		m_pLibSMB2FH = NULL;
	}

	// Destroy URL
	if(m_pLibSMB2Url)
	{
		smb2_destroy_url(m_pLibSMB2Url);
		m_pLibSMB2Url = NULL;
	}

	m_FileSize = 0;
	// Note: Connection stays alive for reuse
}

void CXBLibSMB2::Close()
{
	// First close any open handles
	CloseHandle();

	// Then disconnect and destroy context
	if(m_pLibSMB2Context)
	{
		smb2_disconnect_share(m_pLibSMB2Context);
		smb2_destroy_context(m_pLibSMB2Context);
		m_pLibSMB2Context = NULL;
	}

	m_ConnectedServer.Empty();
	m_ConnectedShare.Empty();
}

// ---------------------------------------------------------------------------
// Per-instance file handle API
// These methods allow multiple files to be open concurrently on the shared
// SMB connection.  Each CFileSMB instance owns its own smb2fh* and calls
// these through the global CXBLibSMB2 (which serializes access via its
// CCriticalSection base class).
// ---------------------------------------------------------------------------

bool CXBLibSMB2::EnsureConnection(const CURL& url)
{
	// Create context if needed
	if(!m_pLibSMB2Context)
	{
		m_pLibSMB2Context = smb2_init_context();

		if(m_pLibSMB2Context == NULL)
		{
			CLog::Log(LOGERROR, "CXBLibSMB2::EnsureConnection - Failed to init context");
			return false;
		}
	}

	smb2_set_user(m_pLibSMB2Context, url.GetUserName());
	smb2_set_password(m_pLibSMB2Context, url.GetPassWord());
	smb2_set_domain(m_pLibSMB2Context, "workgroup");

	struct smb2_url* pUrl = smb2_parse_url(m_pLibSMB2Context, "smb://"+url.GetHostName()+":445/"+url.GetShareName());

	if(!pUrl)
	{
		CLog::Log(LOGERROR, "CXBLibSMB2::EnsureConnection - Failed to parse url: %s", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	smb2_set_security_mode(m_pLibSMB2Context, SMB2_NEGOTIATE_SIGNING_ENABLED);

	if(!IsConnectedToShare(pUrl->server, pUrl->share))
	{
		if(!m_ConnectedServer.IsEmpty())
		{
			smb2_disconnect_share(m_pLibSMB2Context);
			m_ConnectedServer.Empty();
			m_ConnectedShare.Empty();
		}

		if(smb2_connect_share(m_pLibSMB2Context, pUrl->server, pUrl->share, pUrl->user) != 0)
		{
			CLog::Log(LOGERROR, "CXBLibSMB2::EnsureConnection - smb2_connect_share failed. %s", smb2_get_error(m_pLibSMB2Context));
			smb2_destroy_url(pUrl);
			return false;
		}
		m_ConnectedServer = pUrl->server;
		m_ConnectedShare = pUrl->share;
	}

	smb2_destroy_url(pUrl);
	return true;
}

smb2fh* CXBLibSMB2::FileOpen(const CURL& url, UINT64& outFileSize)
{
	outFileSize = 0;

	if(!EnsureConnection(url))
		return NULL;

	struct smb2_url* pUrl = smb2_parse_url(m_pLibSMB2Context, "smb://"+url.GetHostName()+":445/"+url.GetShareName());

	if(!pUrl)
	{
		CLog::Log(LOGERROR, "CXBLibSMB2::FileOpen - Failed to parse url: %s", smb2_get_error(m_pLibSMB2Context));
		return NULL;
	}

	struct smb2fh* fh = smb2_open(m_pLibSMB2Context, pUrl->path, O_RDONLY | O_BINARY);
	smb2_destroy_url(pUrl);

	if(fh == NULL)
		return NULL;

	outFileSize = smb2_lseek(m_pLibSMB2Context, fh, 0, SEEK_END, NULL);
	smb2_lseek(m_pLibSMB2Context, fh, 0, SEEK_SET, NULL);

	return fh;
}

unsigned int CXBLibSMB2::FileRead(smb2fh* fh, void* lpBuf, __int64 uiBufSize)
{
	if(fh == NULL || m_pLibSMB2Context == NULL)
		return 0;

	int bytesRead = smb2_read(m_pLibSMB2Context, fh, static_cast<uint8_t*>(lpBuf), (unsigned int)uiBufSize);

	if(bytesRead < 0)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - smb2_read returned error %i", errno);
		return 0;
	}

	return (unsigned int)bytesRead;
}

__int64 CXBLibSMB2::FileSeek(smb2fh* fh, __int64 iFilePosition, int iWhence)
{
	if(fh == NULL || m_pLibSMB2Context == NULL)
		return -1;

	INT64 iPos = smb2_lseek(m_pLibSMB2Context, fh, iFilePosition, iWhence, NULL);

	if(iPos < 0)
		return -1;

	return (__int64)iPos;
}

__int64 CXBLibSMB2::FileGetPosition(smb2fh* fh)
{
	if(fh == NULL || m_pLibSMB2Context == NULL)
		return 0;

	__int64 iPos = smb2_lseek(m_pLibSMB2Context, fh, 0, SEEK_CUR, NULL);

	if(iPos < 0)
		return 0;

	return iPos;
}

void CXBLibSMB2::FileClose(smb2fh* fh)
{
	if(fh != NULL && m_pLibSMB2Context != NULL)
		smb2_close(m_pLibSMB2Context, fh);
}
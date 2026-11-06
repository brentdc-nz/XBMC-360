#include "XBLibSmb2.h"
#include "utils\Log.h"
#include <fcntl.h>

// Who to sign in as. A plain name is an account on the server itself, as before (domain "workgroup"); a domain
// account can be DOMAIN\name, or name@domain, whose domain is in the name, so NTLM is sent none beside it; and an
// smb://domain;name@server/ URL gives its domain apart.
static void SetCredentials(struct smb2_context* smb2, const CURL& url)
{
	CStdString user = url.GetUserName();
	CStdString domain = url.GetDomain();

	int iSlash = user.Find('\\');
	if (domain.IsEmpty() && iSlash > 0)
	{
		domain = user.Left(iSlash);
		user = user.Mid(iSlash + 1);
	}
	if (domain.IsEmpty() && user.Find('@') < 0)
		domain = "workgroup";

	smb2_set_user(smb2, user.c_str());
	smb2_set_password(smb2, url.GetPassWord().c_str());
	smb2_set_domain(smb2, domain.c_str());
}

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

	SetCredentials(m_pLibSMB2Context, url);
		
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

	SetCredentials(m_pLibSMB2Context, url);
		
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

	SetCredentials(m_pLibSMB2Context, url);

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

// The URL as libsmb2 sees it: server, share and the path within the share.
// The caller frees it with smb2_destroy_url().
struct smb2_url* CXBLibSMB2::ParseUrl(const CURL& url)
{
	struct smb2_url* pUrl = smb2_parse_url(m_pLibSMB2Context, "smb://"+url.GetHostName()+":445/"+url.GetShareName());

	if(!pUrl)
		CLog::Log(LOGERROR, "CXBLibSMB2::ParseUrl - Failed to parse url: %s", smb2_get_error(m_pLibSMB2Context));

	return pUrl;
}

// The path within the share as SMB names it: empty for the share itself
// (libsmb2 leaves it NULL) and without the trailing slash a folder URL has
static CStdString PathInShare(const struct smb2_url* pUrl)
{
	CStdString strPath(pUrl->path ? pUrl->path : "");
	strPath.TrimRight('/');
	return strPath;
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

smb2fh* CXBLibSMB2::FileOpenForWrite(const CURL& url, bool bOverWrite, UINT64& outFileSize)
{
	outFileSize = 0;

	if(!EnsureConnection(url))
		return NULL;

	struct smb2_url* pUrl = ParseUrl(url);

	if(!pUrl)
		return NULL;

	// As CFileHD does: create the file if it's missing, and empty it only when overwriting
	int flags = O_RDWR | O_CREAT | O_BINARY;
	if(bOverWrite)
		flags |= O_TRUNC;

	struct smb2fh* fh = smb2_open(m_pLibSMB2Context, pUrl->path, flags);
	smb2_destroy_url(pUrl);

	if(fh == NULL)
	{
		CLog::Log(LOGERROR, "CXBLibSMB2::FileOpenForWrite - smb2_open failed: %s", smb2_get_error(m_pLibSMB2Context));
		return NULL;
	}

	outFileSize = smb2_lseek(m_pLibSMB2Context, fh, 0, SEEK_END, NULL);
	smb2_lseek(m_pLibSMB2Context, fh, 0, SEEK_SET, NULL);

	return fh;
}

int CXBLibSMB2::FileWrite(smb2fh* fh, const void* lpBuf, __int64 uiBufSize)
{
	if(fh == NULL || m_pLibSMB2Context == NULL)
		return 0;

	// A request carries no more than the server's max write size, so
	// smb2_write can write less than it's given: go on until it's all written
	const uint8_t* pBuf = static_cast<const uint8_t*>(lpBuf);
	__int64 iWritten = 0;

	while(iWritten < uiBufSize)
	{
		int bytesWritten = smb2_write(m_pLibSMB2Context, fh, pBuf + iWritten, (uint32_t)(uiBufSize - iWritten));

		if(bytesWritten <= 0)
		{
			CLog::Log(LOGERROR, __FUNCTION__" - smb2_write failed: %s", smb2_get_error(m_pLibSMB2Context));
			break;
		}

		iWritten += bytesWritten;
	}

	return (int)iWritten;
}

bool CXBLibSMB2::Stat(const CURL& url, struct smb2_stat_64& st)
{
	if(!EnsureConnection(url))
		return false;

	struct smb2_url* pUrl = ParseUrl(url);

	if(!pUrl)
		return false;

	// Not logged when it fails: asking whether something exists is often expected to
	int rc = smb2_stat(m_pLibSMB2Context, PathInShare(pUrl).c_str(), &st);
	smb2_destroy_url(pUrl);

	return rc == 0;
}

bool CXBLibSMB2::Delete(const CURL& url)
{
	if(!EnsureConnection(url))
		return false;

	struct smb2_url* pUrl = ParseUrl(url);

	if(!pUrl)
		return false;

	int rc = smb2_unlink(m_pLibSMB2Context, PathInShare(pUrl).c_str());
	smb2_destroy_url(pUrl);

	if(rc != 0)
	{
		CLog::Log(LOGERROR, "CXBLibSMB2::Delete - smb2_unlink failed: %s", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	return true;
}

bool CXBLibSMB2::Rename(const CURL& url, const CURL& urlNew)
{
	if(!EnsureConnection(url))
		return false;

	struct smb2_url* pUrl = ParseUrl(url);
	struct smb2_url* pUrlNew = ParseUrl(urlNew);
	bool bRenamed = false;

	if(pUrl && pUrlNew)
	{
		// SMB renames within a share; moving elsewhere is a copy
		if(CStdString(pUrl->server).CompareNoCase(pUrlNew->server) != 0 || CStdString(pUrl->share).CompareNoCase(pUrlNew->share) != 0)
			CLog::Log(LOGERROR, "CXBLibSMB2::Rename - Can't rename across shares");
		else if(smb2_rename(m_pLibSMB2Context, PathInShare(pUrl).c_str(), PathInShare(pUrlNew).c_str()) != 0)
			CLog::Log(LOGERROR, "CXBLibSMB2::Rename - smb2_rename failed: %s", smb2_get_error(m_pLibSMB2Context));
		else
			bRenamed = true;
	}

	if(pUrl)
		smb2_destroy_url(pUrl);
	if(pUrlNew)
		smb2_destroy_url(pUrlNew);

	return bRenamed;
}

bool CXBLibSMB2::CreateDir(const CURL& url)
{
	if(!EnsureConnection(url))
		return false;

	struct smb2_url* pUrl = ParseUrl(url);

	if(!pUrl)
		return false;

	int rc = smb2_mkdir(m_pLibSMB2Context, PathInShare(pUrl).c_str());
	smb2_destroy_url(pUrl);

	if(rc != 0)
	{
		CLog::Log(LOGERROR, "CXBLibSMB2::CreateDir - smb2_mkdir failed: %s", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	return true;
}

bool CXBLibSMB2::RemoveDir(const CURL& url)
{
	if(!EnsureConnection(url))
		return false;

	struct smb2_url* pUrl = ParseUrl(url);

	if(!pUrl)
		return false;

	int rc = smb2_rmdir(m_pLibSMB2Context, PathInShare(pUrl).c_str());
	smb2_destroy_url(pUrl);

	if(rc != 0)
	{
		CLog::Log(LOGERROR, "CXBLibSMB2::RemoveDir - smb2_rmdir failed: %s", smb2_get_error(m_pLibSMB2Context));
		return false;
	}

	return true;
}
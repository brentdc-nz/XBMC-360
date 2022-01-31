#include "FileSMB.h"
#include "..\utils\SingleLock.h"
#include "..\utils\log.h"
#include <fcntl.h>

using namespace XFILE;

CLibSMB2Wrap::CLibSMB2Wrap()
{
	m_pLibSMB2Context = NULL;
	m_pLibSMB2FH = NULL;
	m_pUrl = NULL;
	m_iFileSize = 0;
}

CLibSMB2Wrap::~CLibSMB2Wrap()
{
}

void CLibSMB2Wrap::Init()
{
	CSingleLock(*this);

	if(!m_pLibSMB2Context)
	{
		m_pLibSMB2Context = smb2_init_context();

		if(m_pLibSMB2Context == NULL)
		{
			CLog::Log(LOGERROR, "Failed to init context\n");
			return;
		}

#if 1 //TEST DATA - WIP to get from GUI
		smb2_set_user(m_pLibSMB2Context, "Administrator");		
		smb2_set_password(m_pLibSMB2Context, "xyz");
		smb2_set_domain(m_pLibSMB2Context, "workgroup");
		
		m_pUrl = smb2_parse_url(m_pLibSMB2Context, "smb://192.168.178.145:445/Storage/Videos/Movies/Heat/Heat [1995][Lemavik].avi");
#endif

		if(m_pUrl == NULL)
		{
			CLog::Log(LOGERROR, "Failed to parse url: %s\n", smb2_get_error(m_pLibSMB2Context));
			return;
		}

		smb2_set_security_mode(m_pLibSMB2Context, SMB2_NEGOTIATE_SIGNING_ENABLED);

		// Connect to the share 
		if(smb2_connect_share(m_pLibSMB2Context, m_pUrl->server, m_pUrl->share, m_pUrl->user) != 0)
		{
			CLog::Log(LOGERROR, "smb2_connect_share failed. %s", smb2_get_error(m_pLibSMB2Context));
			return;
		}
	}
}

bool CLibSMB2Wrap::OpenFile()
{
	if(m_pLibSMB2Context == NULL)
		return false;

	m_pLibSMB2FH = smb2_open(m_pLibSMB2Context, m_pUrl->path, O_RDONLY | O_BINARY);

	if(m_pLibSMB2FH == NULL)
		return false;

	UINT64 ret = smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, 0, SEEK_END, NULL);

	m_iFileSize = ret;

	// We've opened the file!
	return true;
}

__int64 CLibSMB2Wrap::Seek(__int64 iFilePosition, int iWhence)
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return -1;

	CSingleLock lock(*this);

	INT64 iPos = smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, iFilePosition, iWhence, NULL);

	if(iPos < 0)
		return -1;

	return(__int64)iPos;
}

unsigned int CLibSMB2Wrap::Read(void *lpBuf, __int64 uiBufSize)
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return 0;

	CSingleLock lock(*this);

	int bytesRead = smb2_read(m_pLibSMB2Context, m_pLibSMB2FH,static_cast<uint8_t *>(lpBuf)/* (unsigned __int8*)lpBuf*/, (unsigned int)uiBufSize);

	if(bytesRead < 0)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - smbc_read returned error %i", errno);
		return 0;
	}

	return (unsigned int)bytesRead;
}

__int64 CLibSMB2Wrap::GetLength()
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return 0;

	return m_iFileSize;
}

__int64 CLibSMB2Wrap::GetPosition()
{
	if(m_pLibSMB2FH == NULL || m_pLibSMB2Context == NULL)
		return 0;

	CSingleLock lock(*this);

	__int64 iPos = smb2_lseek(m_pLibSMB2Context, m_pLibSMB2FH, 0, SEEK_CUR, NULL);
	
	if(iPos < 0)
		return 0;
	
	return iPos;
}

void CLibSMB2Wrap::Close()
{
	if(m_pLibSMB2FH)
	{
		smb2_close(m_pLibSMB2Context, m_pLibSMB2FH);
		m_pLibSMB2Context = NULL;
	}
	
	if(m_pLibSMB2Context)
	{
		smb2_disconnect_share(m_pLibSMB2Context);
		m_pLibSMB2Context = NULL;
	}

	if(m_pUrl)
	{
		smb2_destroy_url(m_pUrl);
		m_pUrl = NULL;
	}

	if(m_pLibSMB2Context)
	{
		smb2_destroy_context(m_pLibSMB2Context);
		m_pLibSMB2Context = NULL;
	}
}

//=====================================================================================

CLibSMB2Wrap g_LibSMB2Wrap;

CFileSMB::CFileSMB()
{
	g_LibSMB2Wrap.Init();
}

CFileSMB::~CFileSMB()
{
}

bool CFileSMB::Open(const CURL& strURL, bool bBinary)
{
	return g_LibSMB2Wrap.OpenFile();
}

bool CFileSMB::OpenForWrite(const CURL& strURL, bool bOverWrite)
{
	CLog::Log(LOGERROR, "CFileSMB OpenForWrite not yet supported.");
	return false; // TODO!!
}

__int64 CFileSMB::GetLength()
{
	return g_LibSMB2Wrap.GetLength();
}

__int64 CFileSMB::GetPosition()
{
	return g_LibSMB2Wrap.GetPosition();
}

void CFileSMB::Close()
{
	g_LibSMB2Wrap.Close();
}

unsigned int CFileSMB::Read(void *lpBuf, __int64 uiBufSize)
{
	return g_LibSMB2Wrap.Read(lpBuf, uiBufSize);
}
	
__int64 CFileSMB::Seek(__int64 iFilePosition, int iWhence)
{
	return g_LibSMB2Wrap.Seek(iFilePosition, iWhence);
}

int CFileSMB::Write(const void* lpBuf, __int64 uiBufSize)
{
	// TODO!
	return 0;
}

int CFileSMB::Stat(const CURL& url, struct __stat64* buffer)
{
	// TODO!!
	return 0;
}

bool CFileSMB::Exists(const CStdString& strPath)
{
	// TODO!!
	return false;
}
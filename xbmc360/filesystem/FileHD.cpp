#include "FileHD.h"
#include "..\utils\Stdafx.h"

using namespace XFILE;

CFileHD::CFileHD() : m_hFile(INVALID_HANDLE_VALUE)
{
}

CFileHD::~CFileHD()
{
	if(m_hFile != INVALID_HANDLE_VALUE) Close();
}

bool CFileHD::Open(const CURL& strURL, bool bBinary)
{
	CStdString strFile = GetLocal(strURL);

	m_hFile.attach(CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL));
	if (!m_hFile.isValid()) return false;

	m_i64FilePos = 0;
	LARGE_INTEGER i64Size;
	GetFileSizeEx((HANDLE)m_hFile, &i64Size);
	m_i64FileLength = i64Size.QuadPart;
	Seek(0, SEEK_SET);

	return true;
}

bool CFileHD::OpenForWrite(const CURL& strURL, bool bOverWrite)
{
	// make sure it's a legal FATX filename (we are writing to the harddisk)
	CStdString strPath = GetLocal(strURL);

	m_hFile.attach(CreateFile(strPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, bOverWrite ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));

	if(!m_hFile.isValid())
		return false;

	m_i64FilePos = 0;
	LARGE_INTEGER i64Size;
	GetFileSizeEx((HANDLE)m_hFile, &i64Size);
	m_i64FileLength = i64Size.QuadPart;
	Seek(0, SEEK_SET);

	return true;
}

__int64 CFileHD::GetLength()
{
	LARGE_INTEGER i64Size;
	GetFileSizeEx((HANDLE)m_hFile, &i64Size);
	return i64Size.QuadPart;
}

__int64 CFileHD::GetPosition()
{
	return m_i64FilePos;
}

void CFileHD::Close()
{
	m_hFile.reset();
}

unsigned int CFileHD::Read(void *lpBuf, __int64 uiBufSize)
{
	if (!m_hFile.isValid()) return 0;
	DWORD nBytesRead;
	
	if ( ReadFile((HANDLE)m_hFile, lpBuf, (DWORD)uiBufSize, &nBytesRead, NULL) )
	{
		m_i64FilePos += nBytesRead;
		return nBytesRead;
	}

	return 0;
}

int CFileHD::Write(const void *lpBuf, __int64 uiBufSize)
{
	if(!m_hFile.isValid())
		return 0;
  
	DWORD nBytesWriten;

	if(WriteFile((HANDLE)m_hFile, lpBuf, (DWORD)uiBufSize, &nBytesWriten, NULL))
		return nBytesWriten;
  
	return 0;
}
	
__int64 CFileHD::Seek(__int64 iFilePosition, int iWhence)
{
	LARGE_INTEGER lPos, lNewPos;
	lPos.QuadPart = iFilePosition;
	int bSuccess;

	switch(iWhence)
	{
	case SEEK_SET:
		if(iFilePosition <= GetLength())
			bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_BEGIN);
		else
			bSuccess = false;
	break;

	case SEEK_CUR:
		if((GetPosition()+iFilePosition) <= GetLength())
			bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_CURRENT);
		else
			bSuccess = false;
	break;

	case SEEK_END:
		bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_END);
    break;
	}
	
	if(bSuccess)
	{
		m_i64FilePos = lNewPos.QuadPart;
		return m_i64FilePos;
	}
	else
		return -1;
}

bool CFileHD::Exists(const CStdString& strPath)
{
	if (strPath.size()==0) return false;

	FILE *fd;
	fd = fopen(strPath.c_str(), "rb");

	if (fd != NULL)
	{
		fclose(fd);
		return true;
	}

	return false;
}

int CFileHD::Stat(const CURL& url, struct __stat64* buffer)
{
//	CStdString strFile = GetLocal(url);
//	return _stat64(strFile.c_str(), buffer); // TODO : Missing in Xbox 360 SDK

	buffer->st_dev = 4294967280;
	buffer->st_ino = 0;
	buffer->st_mode = 16895;
	buffer->st_nlink = 1;
	buffer->st_uid = 0;
	buffer->st_gid = 0;
	buffer->st_rdev = 4294967280;
	buffer->st_size = 0;
	buffer->st_atime = 1000000000;
	buffer->st_mtime = 1000000000;
	buffer->st_ctime = 1000000000;

	return 0;
}

CStdString CFileHD::GetLocal(const CURL &url)
{
	CStdString path(url.GetFileName());
	path.Replace('/', '\\');

	return path;
}
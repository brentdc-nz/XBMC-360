#ifndef H_CXBLIBSMB2
#define H_CXBLIBSMB2

#include "url.h"
#include "utils\CriticalSection.h"
#include "utils\StdString.h"
extern "C" {
#include "libsmb2.h"
#include "smb2.h"
}

class CXBLibSMB2 : public CCriticalSection
{
public:
	CXBLibSMB2();
	~CXBLibSMB2();

	bool Init();

	bool OpenDir(const CURL& url);
	smb2dirent* ReadDir();

	bool OpenFile(const CURL& url);
	__int64 Seek(__int64 iFilePosition, int iWhence);
	unsigned int Read(void *lpBuf, __int64 uiBufSize);
	__int64 GetLength();
	__int64 GetPosition();
	void Close();
	void CloseHandle(); // Close file/dir handle but keep connection alive

	// Per-instance file handle API — allows multiple concurrent open files
	// Caller owns the returned smb2fh* and must close it via FileClose()
	smb2fh* FileOpen(const CURL& url, UINT64& outFileSize);
	unsigned int FileRead(smb2fh* fh, void* lpBuf, __int64 uiBufSize);
	__int64 FileSeek(smb2fh* fh, __int64 iFilePosition, int iWhence);
	__int64 FileGetPosition(smb2fh* fh);
	void FileClose(smb2fh* fh);

private:
	bool EnsureConnection(const CURL& url);
	bool IsConnectedToShare(const char* server, const char* share);
	
	struct smb2_context*	m_pLibSMB2Context;
	struct smb2dir*			m_pLibSMB2H;
	struct smb2_url*		m_pLibSMB2Url;
	struct smb2fh*			m_pLibSMB2FH;

	CStdString m_ConnectedServer;
	CStdString m_ConnectedShare;
	UINT64 m_FileSize;
};

#endif //H_CXBLIBSMB2

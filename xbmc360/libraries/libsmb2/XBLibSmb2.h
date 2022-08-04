#ifndef H_CXBLIBSMB2
#define H_CXBLIBSMB2

#include "url.h"
#include "utils\CriticalSection.h"
#include "utils\Log.h"

extern "C" 
{
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
	unsigned int Write(void *lpBuf, __int64 uiBufSize);
	int Stat(const CURL &url);
private:
	struct smb2_context*	m_pLibSMB2Context;
	struct smb2dir*			m_pLibSMB2H;
	struct smb2_url*		m_pLibSMB2Url;
	struct smb2fh*			m_pLibSMB2FH;
	struct smb2_stat_64*    m_pLibSMB2ST;
	UINT64 m_FileSize;
};

#endif //H_CXBLIBSMB2

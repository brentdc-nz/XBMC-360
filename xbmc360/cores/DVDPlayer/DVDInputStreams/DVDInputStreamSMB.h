#ifndef H_CDVDINPUTSTREAMSMB
#define H_CDVDINPUTSTREAMSMB

#include "DVDInputStream.h"

class CDVDInputStreamSMB : public CDVDInputStream
{
public:
	CDVDInputStreamSMB();
	virtual ~CDVDInputStreamSMB();
	virtual bool Open(const char* strFile, const std::string &content);
	virtual void Close();
	virtual int Read(BYTE* buf, int buf_size);
	virtual __int64 Seek(__int64 offset, int whence);
	virtual bool Pause(double dTime) { return false; };
	virtual bool IsEOF();
	virtual __int64 GetLength();

protected:
	XFILE::CFile* m_pFile;
	bool m_eof;
};

#endif //H_CDVDINPUTSTREAMSMB
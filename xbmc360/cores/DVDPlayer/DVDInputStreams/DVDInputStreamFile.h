#ifndef H_DVDINPUTSTREAMFILE
#define H_DVDINPUTSTREAMFILE

#include "DVDInputStream.h"
#include "filesystem\File.h"

class CDVDInputStreamFile : public CDVDInputStream
{
public:
	CDVDInputStreamFile();
	virtual ~CDVDInputStreamFile();
	virtual bool Open(const char* strFile, const std::string &content);
	virtual void Close();
	virtual int64_t GetLength();
	virtual BitstreamStats GetBitstreamStats() const;
	virtual __int64 Seek(__int64 offset, int whence);
	virtual bool Pause(double dTime) { return false; };
	virtual int Read(BYTE* buf, int buf_size);
	virtual int GetBlockSize();
	virtual bool IsEOF();

protected:
	XFILE::CFile* m_pFile;
	bool m_eof;
};

#endif //H_DVDINPUTSTREAMFILE

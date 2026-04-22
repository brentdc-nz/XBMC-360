#ifndef H_CFILE
#define H_CFILE

#include "FileBase.h"
#include "utils\StdString.h"
#include "utils\BitstreamStats.h"

// Indicate that caller can handle truncated reads, where function returns before entire buffer has been filled
#define READ_TRUNCATED 0x01

// Indicate that caller supports read in the minimum defined chunk size, this disables internal cache then
#define READ_CHUNKED   0x02

// Use cache to access this file
#define READ_CACHED     0x04

// Open without caching, regardless of file type
#define READ_NO_CACHE  0x08

// Calculate bitrate for file while reading
#define READ_BITRATE   0x10

namespace XFILE
{

class CFileStreamBuffer;

class CFile
{
public:
	CFile();
	virtual ~CFile();

	bool Open(const CStdString& strURLFile, unsigned int flags = 0);
	bool OpenForWrite(const CStdString& strFileName, bool bOverWrite = false);
	unsigned int Read(void* lpBuf, int64_t uiBufSize);
	int Write(const void* lpBuf, int64_t uiBufSize);
	__int64 GetPosition();
	__int64 Seek(__int64 iFilePosition, int iWhence = SEEK_SET);
	__int64 GetLength();
	int GetChunkSize() { if (m_pFile) return m_pFile->GetChunkSize(); return 0; }
	BitstreamStats* GetBitstreamStats() { return m_bitStreamStats; }
	void Close();

	// Will return a size, that is aligned to chunk size
	// but always greater or equal to the file's chunk size
	static int GetChunkSize(int chunk, int minimum)
	{
		if(chunk)
			return chunk * ((minimum + chunk - 1) / chunk);
		else
			return minimum;
	}

	static bool Exists(const CStdString& strFileName);
	static bool Cache(const CStdString& strFileName, const CStdString& strDest);
	static int  Stat(const CStdString& strFileName, struct __stat64* buffer);

private:
	unsigned int m_flags;
	CFileBase* m_pFile;
	CFileStreamBuffer* m_pBuffer;
	BitstreamStats* m_bitStreamStats;
};

// streambuf for file io, only supports buffered input currently
class CFileStreamBuffer
	: public std::streambuf
{
public:
	~CFileStreamBuffer();
	CFileStreamBuffer(int backsize = 0);

	void Attach(CFileBase *file);
	void Detach();

private:
	virtual int_type underflow();
	virtual std::streamsize showmanyc();
	virtual pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode = std::ios_base::in | std::ios_base::out);
	virtual pos_type seekpos(pos_type, std::ios_base::openmode = std::ios_base::in | std::ios_base::out);

	CFileBase* m_file;
	char*  m_buffer;
	int    m_backsize;
	int    m_frontsize;
};

};
#endif //H_CFILE
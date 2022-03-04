#ifndef H_DVDINPUTSTREAM
#define H_DVDINPUTSTREAM

#include "filesystem\File.h"
#include "FileItem.h"
#include "utils\BitstreamStats.h"
#include "url.h"

enum DVDStreamType
{
	DVDSTREAM_TYPE_NONE   = -1,
	DVDSTREAM_TYPE_FILE   = 1,
	DVDSTREAM_TYPE_DVD    = 2,
	DVDSTREAM_TYPE_HTTP   = 3,
	DVDSTREAM_TYPE_MEMORY = 4,
	DVDSTREAM_TYPE_FFMPEG = 5,
	DVDSTREAM_TYPE_TV     = 6,
	DVDSTREAM_TYPE_RTMP   = 7,
	DVDSTREAM_TYPE_HTSP   = 8,
};

#define DVDSTREAM_BLOCK_SIZE_FILE (2048 * 16)
#define DVDSTREAM_BLOCK_SIZE_DVD  2048

class CDVDInputStream
{
public:
	class IDisplayTime
	{
    public:
		virtual ~IDisplayTime() {};
		virtual int GetTotalTime() = 0;
		virtual int GetTime() = 0;
	};

	class ISeekTime
	{
	public:
		virtual ~ISeekTime() {};
		virtual bool SeekTime(int ms) = 0;
	};

	CDVDInputStream(DVDStreamType m_streamType);
	virtual ~CDVDInputStream();

	virtual bool Open(const char* strFileName, const std::string& content);
	virtual void Close() = 0;
	void SetFileItem(const CFileItem& item);
	virtual bool NextStream() { return false; }
	virtual std::string& GetContent() { return m_content; };
	virtual std::string& GetFileName() { return m_strFileName; }
	bool IsStreamType(DVDStreamType type) const { return m_streamType == type; }
	virtual __int64 GetLength() = 0;
	virtual BitstreamStats GetBitstreamStats() const { return m_stats; }
	virtual __int64 Seek(__int64 offset, int whence) = 0;
	virtual bool Pause(double dTime) = 0;
	virtual int Read(BYTE* buf, int buf_size) = 0;
	virtual int GetBlockSize() { return 0; }
	virtual bool IsEOF() = 0;  
	virtual int GetCurrentGroupId() { return 0; }

protected:
	DVDStreamType m_streamType;
	std::string m_strFileName;
	BitstreamStats m_stats;
	std::string m_content;
	CFileItem m_item;
};

#endif //H_DVDINPUTSTREAM
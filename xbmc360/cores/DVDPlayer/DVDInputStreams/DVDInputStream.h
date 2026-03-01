#ifndef H_DVDINPUTSTREAM
#define H_DVDINPUTSTREAM

#include "url.h"
#include "utils\BitstreamStats.h"
#include "filesystem\File.h"

#include "FileItem.h"

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

#define SEEK_POSSIBLE 0x10 // flag used to check if protocol allows seeks

#define DVDSTREAM_BLOCK_SIZE_FILE (2048 * 16)
#define DVDSTREAM_BLOCK_SIZE_DVD  2048

namespace XFILE
{
  struct SCacheStatus
  {
    __int64 forward;  /**< number of bytes cached forward of current position */
    unsigned currate;      /**< current read rate from source file since last position change */
    unsigned maxrate;      /**< maximum read rate from source file (at curved position) */
    bool     full;         /**< is the fill level of the cache at the maximum? */
  };
}

class CPoint;

class CDVDInputStream
{
public:
	class IChannel
	{
	public:
		virtual ~IChannel() {};
		virtual bool NextChannel() = 0;
		virtual bool PrevChannel() = 0;
		virtual bool SelectChannel(unsigned int channel) = 0;
		virtual bool UpdateItem(CFileItem& item) = 0;
		virtual bool CanPause() = 0;
		virtual bool CanSeek() = 0;
	};

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

	class IChapter
	{
	public:
		virtual ~IChapter() {};
		virtual int  GetChapter() = 0;
		virtual int  GetChapterCount() = 0;
		virtual void GetChapterName(std::string& name) = 0;
		virtual bool SeekChapter(int ch) = 0;
	};

	class IMenus
	{
	public:
		virtual ~IMenus() {};
		virtual void ActivateButton() = 0;
		virtual void SelectButton(int iButton) = 0;
		virtual int  GetCurrentButton() = 0;
		virtual int  GetTotalButtons() = 0;
		virtual void OnUp() = 0;
		virtual void OnDown() = 0;
		virtual void OnLeft() = 0;
		virtual void OnRight() = 0;
		virtual void OnMenu() = 0;
		virtual void OnBack() = 0;
		virtual void OnNext() = 0;
		virtual void OnPrevious() = 0;
		virtual bool OnMouseMove(const CPoint &point) = 0;
		virtual bool OnMouseClick(const CPoint &point) = 0;
		virtual bool IsInMenu() = 0;
		virtual void SkipStill() = 0;
		virtual double GetTimeStampCorrection() = 0;
		virtual bool GetState(std::string &xmlstate) = 0;
		virtual bool SetState(const std::string &xmlstate) = 0;
	};

	class ISeekable
	{
	public:
		virtual ~ISeekable() {};
		virtual bool CanSeek()  = 0;
		virtual bool CanPause() = 0;
	};

	enum ENextStream
	{
		NEXTSTREAM_NONE,
		NEXTSTREAM_OPEN,
		NEXTSTREAM_RETRY,
	};

	CDVDInputStream(DVDStreamType m_streamType);
	virtual ~CDVDInputStream();

	virtual bool Open(const char* strFileName, const std::string& content);
	virtual void Close() = 0;
	void SetFileItem(const CFileItem& item);
	virtual ENextStream NextStream() { return NEXTSTREAM_NONE; }
	virtual void Abort() {}
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

	/*! \brief Indicate expected read rate in bytes per second.
	 *  This could be used to throttle caching rate. Should
	 *  be seen as only a hint
	 */
	virtual void SetReadRate(unsigned rate) {}

	/*! \brief Get the cache status
	 \return true when cache status was successfully obtained
	 */
	virtual bool GetCacheStatus(XFILE::SCacheStatus *status) { return false; }

protected:
	DVDStreamType m_streamType;
	std::string m_strFileName;
	BitstreamStats m_stats;
	std::string m_content;
	CFileItem m_item;
};

#endif //H_DVDINPUTSTREAM
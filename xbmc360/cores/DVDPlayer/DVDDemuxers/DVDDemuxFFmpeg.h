#ifndef H_CDVDDEMUXFFMPEG
#define H_CDVDDEMUXFFMPEG

//FFMPEG
extern "C" 
{
#include "libavformat\AvFormat.h"
#include "libavcodec\AvCodec.h"
}

#undef MAX_STREAMS

#include "DVDDemux.h"
#include "..\DVDInputStreams\DVDInputStream.h"

#define MAX_STREAMS 20

#define FFMPEG_FILE_BUFFER_SIZE   32768 // Default reading size for ffmpeg
#define FFMPEG_DVDNAV_BUFFER_SIZE 2048  // For dvd's

class CDVDDemuxFFmpeg;

class CDemuxStreamVideoFFmpeg
	: public CDemuxStreamVideo
{
	CDVDDemuxFFmpeg *m_parent;
	AVStream*        m_stream;
public:
	CDemuxStreamVideoFFmpeg(CDVDDemuxFFmpeg *parent, AVStream* stream)
    : m_parent(parent)
	, m_stream(stream)
	{}
	virtual void GetStreamInfo(std::string& strInfo);
};


class CDemuxStreamAudioFFmpeg
  : public CDemuxStreamAudio
{
	CDVDDemuxFFmpeg *m_parent;
	AVStream*        m_stream;
public:
	CDemuxStreamAudioFFmpeg(CDVDDemuxFFmpeg *parent, AVStream* stream)
	: m_parent(parent)
	, m_stream(stream)
	{}
	std::string m_description;

	virtual void GetStreamInfo(std::string& strInfo);
	virtual void GetStreamName(std::string& strInfo);
};

class CDemuxStreamSubtitleFFmpeg
  : public CDemuxStreamSubtitle
{
	CDVDDemuxFFmpeg *m_parent;
	AVStream*        m_stream;
public:
	CDemuxStreamSubtitleFFmpeg(CDVDDemuxFFmpeg *parent, AVStream* stream)
	: m_parent(parent)
	, m_stream(stream)
	{}
	std::string m_description;

	virtual void GetStreamInfo(std::string& strInfo);
	virtual void GetStreamName(std::string& strInfo);
};

class CDVDDemuxFFmpeg : public CDVDDemux
{
public:
	CDVDDemuxFFmpeg();
	virtual ~CDVDDemuxFFmpeg();

	bool Open(CDVDInputStream* pInput);
	void Dispose();
	void Reset();
	void Abort();
	void Flush();
	void AddStream(int iId);
	void Lock()   { EnterCriticalSection(&m_critSection); }
	void Unlock() { LeaveCriticalSection(&m_critSection); }
	int GetStreamLength();
	void SetSpeed(int iSpeed);
	virtual std::string GetFileName();
	DemuxPacket* Read();
	bool SeekTime(int time, bool backwords = false, double* startpts = NULL);
	CDemuxStream* GetStream(int iStreamId);
	int GetNrOfStreams();
	int GetChapterCount();
	int GetChapter();
	void GetChapterName(std::string& strChapterName);
	double ConvertTimestamp(int64_t pts, int den, int num);
	void UpdateCurrentPTS();

	CRITICAL_SECTION m_critSection;
	CDemuxStream* m_streams[MAX_STREAMS]; // Maximum number of streams that ffmpeg can handle

	AVIOContext* m_ioContext;

	bool Aborted();

	AVFormatContext* m_pFormatContext;

protected:
	friend class CDemuxStreamAudioFFmpeg;
	friend class CDemuxStreamVideoFFmpeg;
	friend class CDemuxStreamSubtitleFFmpeg;

	double   m_iCurrentPts; // Used for stream length estimation
	bool     m_bMatroska;
	bool     m_bAVI;
	int      m_speed;
	unsigned m_program;
	DWORD    m_timeout;
	unsigned char m_buffer[FFMPEG_FILE_BUFFER_SIZE + AVPROBE_PADDING_SIZE];

	CDVDInputStream* m_pInput;
};

#endif //H_CDVDDEMUXFFMPEG
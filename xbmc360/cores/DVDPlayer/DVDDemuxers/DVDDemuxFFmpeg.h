#ifndef H_CDVDDEMUXFFMPEG
#define H_CDVDDEMUXFFMPEG

extern "C" 
{
#include "libavformat\AvFormat.h"
#include "libavcodec\AvCodec.h"
}

#undef MAX_STREAMS

#include "DVDDemux.h"
#include "..\DVDInputStreams\DVDInputStream.h"

#define MAX_STREAMS 20

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

#define FFMPEG_FILE_BUFFER_SIZE   32768 // default reading size for ffmpeg
#define FFMPEG_DVDNAV_BUFFER_SIZE 2048  // for dvd's

class CDVDDemuxFFmpeg : public CDVDDemux
{
public:
	CDVDDemuxFFmpeg();
	virtual ~CDVDDemuxFFmpeg();

	bool Open(CDVDInputStream* pInput);
	void Dispose();
	void Reset();
	void Flush();
	void Abort();
	void SetSpeed(int iSpeed);
	virtual std::string GetFileName();

	DemuxPacket* Read();

	bool SeekTime(int time, bool backwords = false, double* startpts = NULL);
	bool SeekByte(int64_t pos);
	int GetStreamLength();
	CDemuxStream* GetStream(int iStreamId);
	int GetNrOfStreams();

	bool SeekChapter(int chapter, double* startpts = NULL);
	int GetChapterCount();
	int GetChapter();
	void GetChapterName(std::string& strChapterName);
	virtual void GetStreamCodecName(int iStreamId, CStdString &strName);

	bool Aborted();

	AVFormatContext* m_pFormatContext;

protected:
	friend class CDemuxStreamAudioFFmpeg;
	friend class CDemuxStreamVideoFFmpeg;
	friend class CDemuxStreamSubtitleFFmpeg;

	int ReadFrame(AVPacket *packet);
	void AddStream(int iId);
	void Lock()   { EnterCriticalSection(&m_critSection); }
	void Unlock() { LeaveCriticalSection(&m_critSection); }

	double ConvertTimestamp(int64_t pts, int den, int num);
	void UpdateCurrentPTS();

	CRITICAL_SECTION m_critSection;
	CDemuxStream* m_streams[MAX_STREAMS]; // maximum number of streams that ffmpeg can handle

	AVIOContext* m_ioContext;

	double   m_iCurrentPts; // Used for stream length estimation
	bool     m_bMatroska;
	bool     m_bAVI;
	int      m_speed;
	unsigned m_program;
	DWORD    m_timeout;

	CDVDInputStream* m_pInput;
};

#endif //H_CDVDDEMUXFFMPEG
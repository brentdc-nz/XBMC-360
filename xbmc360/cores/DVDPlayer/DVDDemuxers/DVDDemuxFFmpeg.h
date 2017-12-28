#ifndef H_CDVDDEMUXFFMPEG
#define H_CDVDDEMUXFFMPEG

//FFMPEG
extern "C" 
{
    #ifndef __STDC_CONSTANT_MACROS
    #define __STDC_CONSTANT_MACROS
    #endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "DVDDemux.h"
#include <string>

#define MAX_STREAMS 20

using namespace std;

#include "..\..\..\utils\Thread.h"

class CDemuxStreamVideoFFmpeg : public CDemuxStreamVideo
{
public:
	CDemuxStreamVideoFFmpeg() : CDemuxStreamVideo()
	{
	}
  
	virtual void GetStreamInfo(std::string& strInfo);
};


class CDemuxStreamAudioFFmpeg : public CDemuxStreamAudio
{
public:
	CDemuxStreamAudioFFmpeg() : CDemuxStreamAudio()
	{
		previous_dts = 0LL;
	}

	virtual void GetStreamInfo(std::string& strInfo);
	__int64 previous_dts;
};

class CDVDDemuxFFmpeg : public CDVDDemux
{
public:
	CDVDDemuxFFmpeg();
	~CDVDDemuxFFmpeg();

	virtual bool Open(/*CDVDInputStream* pInput*/string strFileParth);
	virtual void Dispose();

	virtual CDVDDemux::DemuxPacket* Read();

	bool Seek(int iTime);

	AVFormatContext* m_pFormatContext;

	CRITICAL_SECTION m_critSection;
	CDemuxStream* m_streams[20]; // maximum number of streams that ffmpeg can handle

	virtual CDemuxStream* GetStream(int iStreamId);
	virtual int GetNrOfStreams();

private:
	void AddStream(int iId);

	void Lock();
	void Unlock();

	unsigned __int64 m_iCurrentPts; // used for stream length estimation
};

#endif //H_CDVDDEMUXFFMPEG
#ifndef H_CDVDDEMUX
#define H_CDVDDEMUX

#include <string>

class CDVDInputStream;

typedef unsigned char BYTE;

// FFMPEG
extern "C"
{
#include <libavcodec/avcodec.h>
}

enum AVCodecID;

enum StreamType
{
	STREAM_NONE,    // Unknown
	STREAM_AUDIO,   // Audio stream
	STREAM_VIDEO,   // Video stream
	STREAM_DATA,    // Data stream (i.e. DVDs and SPUs)
};

/*
 * CDemuxStream
 * Base class for all demuxer streams
 */
class CDemuxStream
{
public:
	CDemuxStream()
	{
		iId = 0;
		codec = (CodecID)0; // CODEC_ID_NONE
		type = STREAM_NONE;
		iDuration = 0;
		pPrivate = NULL;
		disabled = false;
	}
	~CDemuxStream(){};

	virtual void GetStreamInfo(std::string& strInfo)
	{
		strInfo = "";
	}

	int iId; // file stream id
	CodecID codec;
	StreamType type;
	int iDuration; // in seconds
	void* pPrivate; // private pointer or the demuxer

	bool disabled; // set when stream is disabled. (when no decoder exists)
};

class CDemuxStreamVideo : public CDemuxStream
{
public:
	CDemuxStreamVideo()
	{
		iFpsScale = 0;
		iFpsRate = 0;
		iHeight = 0;
		iWidth = 0;
		type = STREAM_VIDEO;
	}
	~CDemuxStreamVideo(){}

	int iFpsRate;
	int iFpsScale;
	int iWidth;
	int iHeight;

};

class CDemuxStreamAudio : public CDemuxStream
{
public:
	CDemuxStreamAudio()
	{
		iChannels = 0;
		iSampleRate = 0;
		type = STREAM_AUDIO;
	}
	~CDemuxStreamAudio() {};



	int iChannels;
	int iSampleRate;
};

class CDVDDemux
{
public:

	typedef struct DemuxPacket
	{
		BYTE* pData; // data
		int iSize; // data size
		int iStreamId; // integer representing the stream index

		unsigned __int64 pts; // pts in DVD_TIME_BASE
		unsigned __int64 dts; // dts in DVD_TIME_BASE
	}
	DemuxPacket;

	/*
	* Open the demuxer, returns true on success
	*/
	virtual bool Open(CDVDInputStream* pInput) = 0;
	/*
	* Dispose, Free all resources
	*/
	virtual void Dispose() = 0;
	/*
	* returns the total time in msec
	*/
	virtual int GetStreamLenght() = 0;
	/*
	* returns the stream or NULL on error, starting from 0
	*/
	virtual CDemuxStream* GetStream(int iStreamId) = 0;  
	/*
	* return nr of streams, 0 if none
	*/
	virtual int GetNrOfStreams() = 0;
	/*
	* Read a packet, returns NULL on error
	*/
	virtual DemuxPacket* Read() = 0;
	/*
	* Seek, time in msec calculated from stream start
	*/
	virtual bool Seek(int iTime) = 0;

protected:
	CDVDInputStream* m_pInput; // Global stream info
};

#endif //H_CDVDDEMUX
#ifndef H_CDVDDEMUX
#define H_CDVDDEMUX

#include "utils/StdString.h"
//FFMPEG
extern "C" 
{
#include "libavcodec\avcodec.h"
}

enum StreamType
{
	STREAM_NONE,    // If unknown
	STREAM_AUDIO,   // Audio stream
	STREAM_VIDEO,   // Video stream
	STREAM_DATA,    // Data stream
	STREAM_SUBTITLE // Subtitle stream
};

enum StreamSource {
	STREAM_SOURCE_NONE          = 0x000,
	STREAM_SOURCE_DEMUX         = 0x100, 
	STREAM_SOURCE_NAV           = 0x200,
	STREAM_SOURCE_DEMUX_SUB     = 0x300,
	STREAM_SOURCE_TEXT          = 0x400
};

#define STREAM_SOURCE_MASK(a) ((a) & 0xf00)

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
		iPhysicalId = 0;
		codec = (CodecID)0; // CODEC_ID_NONE
		codec_fourcc = 0;
		type = STREAM_NONE;
		source = STREAM_SOURCE_NONE;
		iDuration = 0;
		pPrivate = NULL;
		ExtraData = NULL;
		ExtraSize = 0;
		memset(language, 0, sizeof(language));
		disabled = false;
		flags = FLAG_NONE;
	}

	virtual ~CDemuxStream()
	{
	}

	virtual void GetStreamInfo(std::string& strInfo)
	{
		strInfo = "";
	}

	virtual void GetStreamName(std::string& strInfo);
	virtual void SetDiscard(AVDiscard discard);

	int iId;         // most of the time starting from 0
	int iPhysicalId; // id
	CodecID codec;
	unsigned int codec_fourcc; // if available
	StreamType type;
	int source;

	int iDuration; // In mseconds
	void* pPrivate; // Private pointer for the demuxer
	void* ExtraData; // Extra data for codec to use
	unsigned int ExtraSize; // Size of extra data

	char language[4]; // ISO 639 3-letter language code (empty string if undefined)
	bool disabled; // Set when stream is disabled. (when no decoder exists)

	int  changes; // Increment on change which player may need to know about

	enum EFlags
	{ FLAG_NONE     = 0x0000 
	, FLAG_DEFAULT  = 0x0001
	, FLAG_DUB      = 0x0002
	, FLAG_ORIGINAL = 0x0004
	, FLAG_COMMENT  = 0x0008
	, FLAG_LYRICS   = 0x0010
	, FLAG_KARAOKE  = 0x0020
	, FLAG_FORCED   = 0x0040
	} flags;

};


class CDemuxStreamVideo : public CDemuxStream
{
public:
	CDemuxStreamVideo() : CDemuxStream()
	{
		iFpsScale = 0;
		iFpsRate = 0;
		iHeight = 0;
		iWidth = 0;
		fAspect = 0.0;
		bVFR = false;
		bPTSInvalid = false;
		type = STREAM_VIDEO;
	}

	virtual ~CDemuxStreamVideo() {}
	int iFpsScale; // Scale of 1000 and a rate of 29970 will result in 29.97 fps
	int iFpsRate;
	int iHeight; // Height of the stream reported by the demuxer
	int iWidth; // Width of the stream reported by the demuxer
	float fAspect; // Display aspect of stream
	bool bVFR;  // Variable framerate
	bool bPTSInvalid; // pts cannot be trusted (avi's).
};

class CDemuxStreamAudio : public CDemuxStream
{
public:
	CDemuxStreamAudio() : CDemuxStream()
	{
		iChannels = 0;
		iSampleRate = 0;
		iBlockAlign = 0;
		iBitRate = 0;
		iBitsPerSample = 0;
		type = STREAM_AUDIO;
	}

	 ~CDemuxStreamAudio() {}

	void GetStreamType(std::string& strInfo);

	int iChannels;
	int iSampleRate;
	int iBlockAlign;
	int iBitRate;
	int iBitsPerSample;
};

class CDemuxStreamSubtitle : public CDemuxStream
{
public:
	CDemuxStreamSubtitle() : CDemuxStream()
	{
		identifier = 0;
		type = STREAM_SUBTITLE;
	}

	int identifier;
};

typedef struct DemuxPacket
{
	BYTE* pData;   // Data
	int iSize;     // Data size
	int iStreamId; // Integer representing the stream index
	int iGroupId;  // The group this data belongs to, used to group data from different streams together
  
	double pts; // pts in DVD_TIME_BASE
	double dts; // dts in DVD_TIME_BASE
	double duration; // Duration in DVD_TIME_BASE if available
} DemuxPacket;

class CDVDDemux
{
public:

	CDVDDemux() {}
	virtual ~CDVDDemux() {}

	/*
	* returns the total time in msec
	*/
	virtual int GetStreamLength() = 0;
	
	/*
	* Set the playspeed, if demuxer can handle different
	* speeds of playback
	*/
	virtual void SetSpeed(int iSpeed) = 0;

	/*
	* returns the stream or NULL on error, starting from 0
	*/
	virtual CDemuxStream* GetStream(int iStreamId) = 0;

	/*
	* return nr of streams, 0 if none
	*/
	virtual int GetNrOfStreams() = 0;

	/*
	* returns opened filename
	*/
	virtual std::string GetFileName() = 0;
	
	/*
	* Read a packet, returns NULL on error
	* 
	*/
	virtual DemuxPacket* Read() = 0;
  
	/*
	* Seek, time in msec calculated from stream start
	*/
	virtual bool SeekTime(int time, bool backwords = false, double* startpts = NULL) = 0;

	/*
	* Aborts any internal reading that might be stalling main thread
	* NOTICE - this can be called from another thread
	*/
	virtual void Abort() = 0;

	/*
	* Reset the entire demuxer (same result as closing and opening it)
	*/
	virtual void Reset() = 0;

	/*
	* Get the number of chapters available
	*/
	virtual int GetChapterCount() { return 0; }

	/*
	* Get the name of the current chapter
	*/
	virtual void GetChapterName(std::string& strChapterName) {}

	/*
	* Get current chapter 
	*/
	virtual int GetChapter() { return 0; }
};

#endif //H_CDVDDEMUX
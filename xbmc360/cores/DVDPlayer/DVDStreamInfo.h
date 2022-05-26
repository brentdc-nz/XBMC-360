#ifndef H_CDVDSTREAMINFO
#define H_CDVDSTREAMINFO

#include "DVDDemuxers\DVDDemux.h"

class CDVDStreamInfo
{
public:
	CDVDStreamInfo();
	CDVDStreamInfo(const CDVDStreamInfo &right, bool withextradata = true);
	CDVDStreamInfo(const CDemuxStream &right, bool withextradata = true);
	~CDVDStreamInfo();

	void Clear(); // Clears current information
	bool Equal(const CDVDStreamInfo &right, bool withextradata);
	bool Equal(const CDemuxStream &right, bool withextradata);

	void Assign(const CDVDStreamInfo &right, bool withextradata);
	void Assign(const CDemuxStream &right, bool withextradata);

	CodecID codec;
	StreamType type;

	// VIDEO
	int fpsscale; // Scale of 1000 and a rate of 29970 will result in 29.97 fps
	int fpsrate;
	int height; // Height of the stream reported by the demuxer
	int width; // Width of the stream reported by the demuxer
	float aspect; // Display aspect as reported by demuxer
	bool vfr; // Variable framerate
	bool stills; // There may be odd still frames in video
	bool ptsinvalid;  // pts cannot be trusted (avi's).

	// AUDIO
	int channels;
	int samplerate;
	int bitrate;
	int blockalign;
	int bitspersample;

	// SUBTITLE
	int identifier;

	// CODEC EXTRADATA
	void*        extradata; // Extra data for codec to use
	unsigned int extrasize; // Size of extra data
	unsigned int codec_tag; // Extra identifier hints for decoding

	bool operator==(const CDVDStreamInfo& right)      { return Equal(right, true);}
	bool operator!=(const CDVDStreamInfo& right)      { return !Equal(right, true);}
	void operator=(const CDVDStreamInfo& right)       { Assign(right, true); }

	bool operator==(const CDemuxStream& right)      { return Equal( CDVDStreamInfo(right, true), true);}
	bool operator!=(const CDemuxStream& right)      { return !Equal( CDVDStreamInfo(right, true), true);}
	void operator=(const CDemuxStream& right)      { Assign(right, true); }
};

#endif //H_CDVDSTREAMINFO
#include "DVDStreamInfo.h"

CDVDStreamInfo::CDVDStreamInfo()                                                     { extradata = NULL; Clear(); }
CDVDStreamInfo::CDVDStreamInfo(const CDVDStreamInfo &right, bool withextradata )     { extradata = NULL; Clear(); Assign(right, withextradata); }
CDVDStreamInfo::CDVDStreamInfo(const CDemuxStream &right, bool withextradata )       { extradata = NULL; Clear(); Assign(right, withextradata); }

CDVDStreamInfo::~CDVDStreamInfo()
{
	if( extradata && extrasize ) free(extradata);

	extradata = NULL;
	extrasize = 0;
}

bool CDVDStreamInfo::Equal(const CDVDStreamInfo& right, bool withextradata)
{
	if(codec      != right.codec
	||  type      != right.type
	||  codec_tag != right.codec_tag)
		return false;

	if(withextradata)
	{
		if(extrasize != right.extrasize) return false;
		if(extrasize)
		{
			if( memcmp(extradata, right.extradata, extrasize) != 0 ) return false;
		}
	}

	// VIDEO
	if(fpsscale  != right.fpsscale
	||  fpsrate  != right.fpsrate
	||  height   != right.height
	||  width    != right.width
	||  stills   != right.stills
	||  ptsinvalid != right.ptsinvalid
	||  vfr      != right.vfr) return false;

	// AUDIO
	if(channels       != right.channels
	||  samplerate    != right.samplerate
	||  blockalign    != right.blockalign
	||  bitrate       != right.bitrate
	||  bitspersample != right.bitspersample ) return false;

	// SUBTITLE
	if(identifier != right.identifier) return false;

	return true;
}

bool CDVDStreamInfo::Equal(const CDemuxStream& right, bool withextradata)
{
	CDVDStreamInfo info;
	info.Assign(right, withextradata);
	return Equal(info, withextradata);
}

void CDVDStreamInfo::Clear()
{
	codec = CODEC_ID_NONE;
	type = STREAM_NONE;
	codec_tag  = 0;

	if(extradata && extrasize ) free(extradata);

	extradata = NULL;
	extrasize = 0;

	fpsscale = 0;
	fpsrate  = 0;
	height   = 0;
	width    = 0;
	aspect   = 0.0;
	vfr      = false;
	stills   = false;
	ptsinvalid = false;

	channels   = 0;
	samplerate = 0;
	blockalign = 0;
	bitrate    = 0;
	bitspersample = 0;

	identifier = 0;
}

// ASSIGNMENT
void CDVDStreamInfo::Assign(const CDVDStreamInfo& right, bool withextradata)
{
	codec = right.codec;
	type = right.type;
	codec_tag = right.codec_tag;

	if(extradata && extrasize) free(extradata);

	if(withextradata && right.extrasize)
	{
		extrasize = right.extrasize;
		extradata = malloc(extrasize);
		memcpy(extradata, right.extradata, extrasize);
	}
	else
	{
		extrasize = 0;
		extradata = 0;
	}

	// VIDEO
	fpsscale = right.fpsscale;
	fpsrate  = right.fpsrate;
	height   = right.height;
	width    = right.width;
	aspect   = right.aspect;
	stills   = right.stills;
	ptsinvalid = right.ptsinvalid;

	// AUDIO
	channels      = right.channels;
	samplerate    = right.samplerate;
	blockalign    = right.blockalign;
	 bitrate       = right.bitrate;
	bitspersample = right.bitspersample;

	// SUBTITLE
	identifier = right.identifier;
}

void CDVDStreamInfo::Assign(const CDemuxStream& right, bool withextradata)
{
	Clear();

	codec = right.codec;
	type = right.type;
	codec_tag = right.codec_fourcc;

	if(withextradata && right.ExtraSize)
	{
		extrasize = right.ExtraSize;
		extradata = malloc(extrasize);
		memcpy(extradata, right.ExtraData, extrasize);
	}

	if(right.type == STREAM_AUDIO)
	{
		const CDemuxStreamAudio *stream = static_cast<const CDemuxStreamAudio*>(&right);
		channels      = stream->iChannels;
		samplerate    = stream->iSampleRate;
		blockalign    = stream->iBlockAlign;
		bitrate       = stream->iBitRate;
		bitspersample = stream->iBitsPerSample;
	}
	else if(right.type == STREAM_VIDEO)
	{
		const CDemuxStreamVideo *stream = static_cast<const CDemuxStreamVideo*>(&right);
		fpsscale  = stream->iFpsScale;
		fpsrate   = stream->iFpsRate;
		height    = stream->iHeight;
		width     = stream->iWidth;
		aspect    = stream->fAspect;
		vfr       = stream->bVFR;
		ptsinvalid = stream->bPTSInvalid;
	}
	else if(right.type == STREAM_SUBTITLE)
	{
		const CDemuxStreamSubtitle *stream = static_cast<const CDemuxStreamSubtitle*>(&right);
		identifier = stream->identifier;
	}
}
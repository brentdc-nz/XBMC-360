#ifndef H_DVDVIDEOCODEC
#define H_DVDVIDEOCODEC

#include "..\DVDCodecs.h"
#include "..\..\DVDStreamInfo.h"

// When modifying these structures, make sure you update all codecs accordingly
#define FRAME_TYPE_UNDEF 0
#define FRAME_TYPE_I 1
#define FRAME_TYPE_P 2
#define FRAME_TYPE_B 3
#define FRAME_TYPE_D 4

// Should be entirely filled by all codecs
struct DVDVideoPicture
{  
	double pts; // Timestamp in seconds, used in the CDVDPlayer class to keep track of pts
	double dts;

	BYTE* data[4];      // [4] = Alpha channel, currently not used
	int iLineSize[4];   // [4] = Alpha channel, currently not used

	unsigned int iFlags;
  
	double       iRepeatPicture;
	double       iDuration;
	unsigned int iFrameType         : 4; // See defines above // 1->I, 2->P, 3->B, 0->Undef
	unsigned int color_matrix       : 4;
	unsigned int color_range        : 1; // 1 indicate if we have a full range of color
	int iGroupId;

	int8_t* qscale_table; // Quantization parameters, primarily used by filters
	int qscale_stride;
	int qscale_type;

	unsigned int iWidth;
	unsigned int iHeight;
	unsigned int iDisplayWidth;  // Width of the picture without black bars
	unsigned int iDisplayHeight; // Height of the picture without black bars

	enum EFormat
	{
		FMT_YUV420P = 0,
		FMT_VDPAU,
	} format;
};

struct DVDVideoUserData
{
	BYTE* data;
	int size;
};

#define DVP_FLAG_TOP_FIELD_FIRST    0x00000001
#define DVP_FLAG_REPEAT_TOP_FIELD   0x00000002 // Set to indicate that the top field should be repeated
#define DVP_FLAG_ALLOCATED          0x00000004 // Set to indicate that this has allocated data
#define DVP_FLAG_INTERLACED         0x00000008 // Set to indicate that this frame is interlaced

#define DVP_FLAG_NOSKIP             0x00000010 // Indicate this picture should never be dropped
#define DVP_FLAG_DROPPED            0x00000020 // Indicate that this picture has been dropped in decoder stage, will have no data
#define DVP_FLAG_NOAUTOSYNC         0x00000040 // Disregard any smooth syncing on this picture

// DVP_FLAG 0x00000100 - 0x00000f00 is in use by libmpeg2!

#define DVP_QSCALE_UNKNOWN          0
#define DVP_QSCALE_MPEG1            1
#define DVP_QSCALE_MPEG2            2
#define DVP_QSCALE_H264             3

class CDVDStreamInfo;
class CDVDCodecOption;
typedef std::vector<CDVDCodecOption> CDVDCodecOptions;

// VC_ messages, messages can be combined
#define VC_ERROR    0x00000001  // An error occured, no other messages will be returned
#define VC_BUFFER   0x00000002  // The decoder needs more data
#define VC_PICTURE  0x00000004  // The decoder got a picture, call Decode(NULL, 0) again to parse the rest of the data
#define VC_USERDATA 0x00000008  // The decoder found some userdata,  call Decode(NULL, 0) again to parse the rest of the data
#define VC_FLUSHED  0x00000010  // The decoder lost it's state, we need to restart decoding again

class CDVDVideoCodec
{
public:
	CDVDVideoCodec() {}
	virtual ~CDVDVideoCodec() {}

	/*
	* Open the decoder, returns true on success
	*/
	virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options) = 0;
  
	/*
	* Dispose, Free all resources
	*/
	virtual void Dispose() = 0;

	/*
	* returns one or a combination of VC_ messages
	* pData and iSize can be NULL, this means we should flush the rest of the data.
	*/
	virtual int Decode(BYTE* pData, int iSize, double dts, double pts) = 0;

	/*
	*
	* should return codecs name
	*/
	virtual const char* GetName() = 0;

	/*
	* Reset the decoder.
	* Should be the same as calling Dispose and Open after each other
	*/
	virtual void Reset() = 0;

	/*
	* will be called by video player indicating if a frame will eventually be dropped
	* codec can then skip actually decoding the data, just consume the data set picture headers
	*/
	virtual void SetDropState(bool bDrop) = 0;

	/*
	* returns true if successfull
	* the data is valid until the next Decode call
	*/
	virtual bool GetPicture(DVDVideoPicture* pDvdVideoPicture) = 0;
};

#endif //H_DVDVIDEOCODEC
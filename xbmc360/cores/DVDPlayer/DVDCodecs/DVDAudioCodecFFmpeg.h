#ifndef H_CDVDAUDIOCODECFFMPEG
#define H_CDVDAUDIOCODECFFMPEG

#include "DVDAudioCodec.h"

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

class CDVDAudioCodecFFmpeg : public CDVDAudioCodec
{
public:
	CDVDAudioCodecFFmpeg();
	virtual ~CDVDAudioCodecFFmpeg();

	virtual bool Open(CodecID codecID, int iChannels, int iSampleRate);
	virtual void Dispose();
	virtual int Decode(BYTE* pData, int iSize);
	virtual int GetData(BYTE** dst);
	virtual void Reset();
	virtual int GetChannels();
	virtual int GetSampleRate();
	virtual int GetBitsPerSample();
	virtual const char* GetName() { return "FFmpeg"; }

protected:
	AVCodecContext* m_pCodecContext;

	unsigned char m_buffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	int m_iBufferSize;
	bool m_bOpenedCodec;
};

#endif //H_CDVDAUDIOCODECFFMPEG
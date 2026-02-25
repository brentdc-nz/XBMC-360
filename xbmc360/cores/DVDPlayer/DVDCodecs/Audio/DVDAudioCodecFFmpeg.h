#ifndef H_CDVDAUDIOCODECFFMPEG
#define H_CDVDAUDIOCODECFFMPEG

#include "DVDAudioCodec.h"

extern "C" {
#include "libswresample/swresample.h"
}

class CDVDAudioCodecFFmpeg : public CDVDAudioCodec
{
public:
	CDVDAudioCodecFFmpeg();
	virtual ~CDVDAudioCodecFFmpeg();

	virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options);
	virtual void Dispose();
	virtual int Decode(BYTE* pData, int iSize);
	virtual int GetData(BYTE** dst);
	virtual void Reset();
	virtual int GetChannels();
	virtual int GetSampleRate();
	virtual int GetBitsPerSample();
	virtual const char* GetName() { return "FFmpeg"; }
	virtual int GetBufferSize() { return m_iBuffered; }

protected:
	AVCodecContext* m_pCodecContext;
	AVFrame*        m_pFrame;            // FFmpeg 1.2: decode output goes into AVFrame
	enum AVSampleFormat m_iSampleFormat; // FFmpeg 1.2: renamed from SampleFormat

	BYTE *m_pBuffer1;
	int   m_iBufferSize1;

	BYTE *m_pBuffer2;
	int   m_iBufferSize2;

	int   m_iOutputSize;                 // Decoded audio data size from AVFrame

	bool m_bOpenedCodec;
	int m_iBuffered;

	SwrContext*     m_pConvert;
	enum AVSampleFormat m_iLastSampleFormat;
	int             m_iLastChannels;
	int             m_iLastSampleRate;

#if 0//ndef _HARDLINKED
	DllAvCodec m_dllAvCodec;
	DllAvUtil m_dllAvUtil;
#endif
};

#endif //H_CDVDAUDIOCODECFFMPEG
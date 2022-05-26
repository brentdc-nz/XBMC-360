#ifndef H_CDVDAUDIOCODECFFMPEG
#define H_CDVDAUDIOCODECFFMPEG

#include "DVDAudioCodec.h"

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
//	AVAudioConvert* m_pConvert;;
	enum SampleFormat m_iSampleFormat;

	BYTE *m_pBuffer1;
	int   m_iBufferSize1;

	BYTE *m_pBuffer2;
	int   m_iBufferSize2;

	bool m_bOpenedCodec;
	int m_iBuffered;

#if 0//ndef _HARDLINKED
	DllAvCodec m_dllAvCodec;
	DllAvUtil m_dllAvUtil;
#endif
};

#endif //H_CDVDAUDIOCODECFFMPEG
#ifndef H_CDVDVIDEOCODECFFMPEG
#define H_CDVDVIDEOCODECFFMPEG

#include "utils\stdafx.h"
#include "DVDVideoCodec.h"
#include "..\DVDCodecs.h"

class CDVDVideoCodecFFmpeg : public CDVDVideoCodec
{
public:
	CDVDVideoCodecFFmpeg();
	virtual ~CDVDVideoCodecFFmpeg();

	virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options);  
	virtual void Dispose();
	virtual int Decode(BYTE* pData, int iSize, double dts, double pts);
	virtual void Reset();
	bool GetPictureCommon(DVDVideoPicture* pDvdVideoPicture);
	virtual bool GetPicture(DVDVideoPicture* pDvdVideoPicture);
	virtual void SetDropState(bool bDrop);
	virtual const char* GetName() { return m_name.c_str(); }; // m_name is never changed after open

protected:
	void GetVideoAspect(AVCodecContext* CodecContext, unsigned int& iWidth, unsigned int& iHeight);

	AVFrame* m_pFrame;
	AVCodecContext* m_pCodecContext;
	AVPicture* m_pConvertFrame;

	int m_iPictureWidth;
	int m_iPictureHeight;

	int m_iScreenWidth;
	int m_iScreenHeight;

#if 0 //ndef _HARDLINKED
	DllAvCodec m_dllAvCodec;
	DllAvUtil  m_dllAvUtil;
	DllSwScale m_dllSwScale;
#endif

	std::string m_name;
	double m_dts;
};

#endif //H_CDVDVIDEOCODECFFMPEG
#include "DVDVideoCodec.h"

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

class CDVDVideoCodecFFmpeg : public CDVDVideoCodec
{
public:
	CDVDVideoCodecFFmpeg();
	virtual ~CDVDVideoCodecFFmpeg();

	virtual bool Open(CodecID codecID, int iWidth, int iHeight);

	virtual void Dispose();

	virtual void SetDropState(bool bDrop);

	virtual int Decode(BYTE* pData, int iSize);

	virtual  void Reset();

	virtual bool GetPicture(DVDVideoPicture* pDvdVideoPicture);

private:
	void GetVideoAspect(AVCodecContext* CodecContext, unsigned int& iWidth, unsigned int& iHeight);

	AVCodecContext* m_pCodecContext;
	AVFrame* m_pFrame;

	AVFrame* m_pConvertFrame;

	int m_iPictureWidth;
	int m_iPictureHeight;
	int m_iScreenWidth;
	int m_iScreenHeight;

	struct SwsContext* m_pSwsContext;
};
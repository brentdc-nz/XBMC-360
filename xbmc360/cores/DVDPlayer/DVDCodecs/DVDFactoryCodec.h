#ifndef H_CDVDFACTORYCODEC
#define H_CDVDFACTORYCODEC

#include "DVDVideoCodec.h"
#include "DVDAudioCodec.h"
#include "..\DVDStreamInfo.h"

class CDVDFactoryCodec
{
public:
	static CDVDVideoCodec* CreateVideoCodec(CDVDStreamInfo &hint);
	static CDVDAudioCodec* CreateAudioCodec(CDVDStreamInfo &hint);

	static CDVDVideoCodec* OpenCodec(CDVDVideoCodec* pCodec,  CDVDStreamInfo &hint);
	static CDVDAudioCodec* OpenCodec(CDVDAudioCodec* pCodec,  CDVDStreamInfo &hint);
};

#endif //H_CDVDFACTORYCODEC
#ifndef H_CDVDFACTORYCODEC
#define H_CDVDFACTORYCODEC

#include "DVDAudioCodec.h"
#include "..\DVDStreamInfo.h"

class CDVDFactoryCodec
{
public:
//	static CDVDVideoCodec* CreateVideoCodec(CDVDStreamInfo &hint ); //TODO
	static CDVDAudioCodec* CreateAudioCodec(CDVDStreamInfo &hint );

//	static CDVDVideoCodec* OpenCodec(CDVDVideoCodec* pCodec,  CDVDStreamInfo &hint, CDVDCodecOptions &options ); //TODO
	static CDVDAudioCodec* OpenCodec(CDVDAudioCodec* pCodec,  CDVDStreamInfo &hint );
};

#endif //H_CDVDFACTORYCODEC
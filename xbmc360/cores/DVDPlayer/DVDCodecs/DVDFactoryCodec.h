#ifndef H_CDVDFACTORYCODEC
#define H_CDVDFACTORYCODEC

#include "DVDCodecs.h"
#include "..\DVDStreamInfo.h"
#include "Audio\DVDAudioCodec.h"

class CDVDVideoCodec;

class CDVDFactoryCodec
{
public:
	static CDVDVideoCodec* CreateVideoCodec(CDVDStreamInfo &hint);
	static CDVDAudioCodec* CreateAudioCodec(CDVDStreamInfo &hint);

	static CDVDVideoCodec* OpenCodec(CDVDVideoCodec* pCodec, CDVDStreamInfo &hint, CDVDCodecOptions &options);
	static CDVDAudioCodec* OpenCodec(CDVDAudioCodec* pCodec, CDVDStreamInfo &hint, CDVDCodecOptions &options);
};

#endif //H_CDVDFACTORYCODEC
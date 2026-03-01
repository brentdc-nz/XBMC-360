#ifndef H_CDVDFACTORYCODEC
#define H_CDVDFACTORYCODEC

class CDVDVideoCodec;
class CDVDAudioCodec;
class CDVDOverlayCodec;

class CDemuxStreamVideo;
class CDVDStreamInfo;
class CDVDCodecOption;
class CDVDCodecOptions;

class CDVDFactoryCodec
{
public:
	static CDVDVideoCodec* CreateVideoCodec(CDVDStreamInfo &hint);
	static CDVDAudioCodec* CreateAudioCodec(CDVDStreamInfo &hint);

	static CDVDVideoCodec* OpenCodec(CDVDVideoCodec* pCodec, CDVDStreamInfo &hint, CDVDCodecOptions &options);
	static CDVDAudioCodec* OpenCodec(CDVDAudioCodec* pCodec, CDVDStreamInfo &hint, CDVDCodecOptions &options);
};

#endif //H_CDVDFACTORYCODEC
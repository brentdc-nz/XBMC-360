#include "DVDFactoryCodec.h"
#include "Video\DVDVideoCodecFFmpeg.h"
#include "Audio\DVDAudioCodecFFmpeg.h"
#include "utils\Log.h"

CDVDVideoCodec* CDVDFactoryCodec::CreateVideoCodec(CDVDStreamInfo &hint)
{
	CDVDVideoCodec* pCodec = NULL;
	CDVDCodecOptions options;

/*  // TODO
	// Dvd's have weird still-frames in it, which is not fully supported in ffmpeg
	if(hint.stills && (hint.codec == CODEC_ID_MPEG2VIDEO || hint.codec == CODEC_ID_MPEG1VIDEO))
	{
		if((pCodec = OpenCodec(new CDVDVideoCodecLibMpeg2(), hint, options))) return pCodec;
	}
*/
	// Try to decide if we want to try halfres decoding
	float pixelrate = (float)hint.width*hint.height*hint.fpsrate/hint.fpsscale;
	if(pixelrate > 1400.0f*720.0f*30.0f)
	{
		CLog::Log(LOGINFO, "CDVDFactoryCodec - High video resolution detected %dx%d, trying half resolution decoding ", hint.width, hint.height);    
		options.push_back(CDVDCodecOption("lowres","1"));    
	}

	if((pCodec = OpenCodec(new CDVDVideoCodecFFmpeg(), hint, options))) return pCodec;

	return NULL;
}

CDVDAudioCodec* CDVDFactoryCodec::CreateAudioCodec(CDVDStreamInfo &hint)
{
	CDVDAudioCodec* pCodec = NULL;
	CDVDCodecOptions options;

	pCodec = NULL; //OpenCodec(new CDVDAudioCodecPassthrough(), hint, options); //TODO
	if(pCodec) return pCodec;

	switch(hint.codec)
	{
		case CODEC_ID_AC3:
		{
//			pCodec = OpenCodec(new CDVDAudioCodecLiba52(), hint, options); //TODO
			if(pCodec) return pCodec;
			break;
		}
		case CODEC_ID_DTS:
		{
//			pCodec = OpenCodec(new CDVDAudioCodecLibDts(), hint, options); //TODO
			if(pCodec) return pCodec;
			break;
		}
#ifdef USE_LIBMAD
		case CODEC_ID_MP2:
		case CODEC_ID_MP3:
		{
			pCodec = OpenCodec(new CDVDAudioCodecLibMad(), hint, options);
			if(pCodec) return pCodec;
			break;
		}
#endif
#ifdef USE_LIBFAAD
		case CODEC_ID_AAC:
		//case CODEC_ID_MPEG4AAC:
		{
			pCodec = OpenCodec(new CDVDAudioCodecLibFaad(), hint, options);
			if(pCodec) return pCodec;
			break;
		}
#endif
		case CODEC_ID_PCM_S32LE:
		case CODEC_ID_PCM_S32BE:
		case CODEC_ID_PCM_U32LE:
		case CODEC_ID_PCM_U32BE:
		case CODEC_ID_PCM_S24LE:
		case CODEC_ID_PCM_S24BE:
		case CODEC_ID_PCM_U24LE:
		case CODEC_ID_PCM_U24BE:
		case CODEC_ID_PCM_S24DAUD:
		case CODEC_ID_PCM_S16LE:
		case CODEC_ID_PCM_S16BE:
		case CODEC_ID_PCM_U16LE:
		case CODEC_ID_PCM_U16BE:
		case CODEC_ID_PCM_S8:
		case CODEC_ID_PCM_U8:
		case CODEC_ID_PCM_ALAW:
		case CODEC_ID_PCM_MULAW:
		{
//			pCodec = OpenCodec(new CDVDAudioCodecPcm(), hint, options); //TODO
			if(pCodec) return pCodec;
			break;
		}
	#if 0
		//case CODEC_ID_LPCM_S16BE:
		//case CODEC_ID_LPCM_S20BE:
		case CODEC_ID_LPCM_S24BE:
		{
			pCodec = OpenCodec(new CDVDAudioCodecLPcm(), hint, options);
			if(pCodec) return pCodec;
			break;
		}
	#endif
		default:
		{
			pCodec = NULL;
			break;
		}
	}

	pCodec = OpenCodec(new CDVDAudioCodecFFmpeg(), hint, options);
	if(pCodec) return pCodec;

	return NULL;
}

CDVDVideoCodec* CDVDFactoryCodec::OpenCodec(CDVDVideoCodec* pCodec, CDVDStreamInfo &hints, CDVDCodecOptions &options)
{  
	try
	{
		CLog::Log(LOGDEBUG, "FactoryCodec - Video: %s - Opening", pCodec->GetName());
		if(pCodec->Open(hints, options))
		{
			CLog::Log(LOGDEBUG, "FactoryCodec - Video: %s - Opened", pCodec->GetName());
			return pCodec;
		}

		CLog::Log(LOGDEBUG, "FactoryCodec - Video: %s - Failed", pCodec->GetName());
		pCodec->Dispose();
		delete pCodec;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "FactoryCodec - Video: Failed with exception");
	}

	return NULL;
}

CDVDAudioCodec* CDVDFactoryCodec::OpenCodec(CDVDAudioCodec* pCodec, CDVDStreamInfo &hints, CDVDCodecOptions &options)
{    
	try
	{
		CLog::Log(LOGDEBUG, "FactoryCodec - Audio: %s - Opening", pCodec->GetName());
		if(pCodec->Open( hints, options ))
		{
			CLog::Log(LOGDEBUG, "FactoryCodec - Audio: %s - Opened", pCodec->GetName());
			return pCodec;
		}

		CLog::Log(LOGDEBUG, "FactoryCodec - Audio: %s - Failed", pCodec->GetName());
		pCodec->Dispose();
		delete pCodec;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "FactoryCodec - Audio: Failed with exception");
	}

	return NULL;
}
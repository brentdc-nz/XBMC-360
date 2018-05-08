#include "DVDFactoryCodec.h"
#include "..\..\..\utils\stdafx.h"
#include "..\..\..\utils\Log.h"

#include "DVDAudioCodecFFmpeg.h"

CDVDAudioCodec* CDVDFactoryCodec::CreateAudioCodec( CDVDStreamInfo &hint )
{
  CDVDAudioCodec* pCodec = NULL;

  // TODO: We only have FFmpeg atm... 

  pCodec = OpenCodec( new CDVDAudioCodecFFmpeg(), hint );
  if( pCodec ) return pCodec;

  return NULL;
}

CDVDAudioCodec* CDVDFactoryCodec::OpenCodec(CDVDAudioCodec* pCodec, CDVDStreamInfo &hint  )
{    
	try
	{
		CLog::Log(LOGDEBUG, "FactoryCodec - Audio: %s - Opening", pCodec->GetName());

		if( pCodec->Open( hint.codec, hint.channels, hint.samplerate ) )
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
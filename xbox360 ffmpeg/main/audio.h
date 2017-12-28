#pragma once
#include <xaudio2.h>

//-------------------------------------------------------------------------------------
// 	Audio Synchronization
//-------------------------------------------------------------------------------------
class XAudio2_BufferNotify : public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;

	XAudio2_BufferNotify() {
		hBufferEndEvent = NULL;
		hBufferEndEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	}

	~XAudio2_BufferNotify() {
		CloseHandle( hBufferEndEvent );
		hBufferEndEvent = NULL;
	}

	STDMETHOD_( void, OnBufferEnd ) ( void *pBufferContext ) {
		SetEvent( hBufferEndEvent );
		if(pBufferContext)
			//XPhysicalFree(pBufferContext);
			free(pBufferContext);
	}

	// dummies:
	STDMETHOD_( void, OnVoiceProcessingPassStart ) ( UINT32 BytesRequired ) {}
	STDMETHOD_( void, OnVoiceProcessingPassEnd ) () {}
	STDMETHOD_( void, OnStreamEnd ) () {}
	STDMETHOD_( void, OnBufferStart ) ( void *pBufferContext ) {}
	STDMETHOD_( void, OnLoopEnd ) ( void *pBufferContext ) {}
	STDMETHOD_( void, OnVoiceError ) ( void *pBufferContext, HRESULT Error ) {};
};


HRESULT InitAudio(FFMpegData * pFFMpegData);
HRESULT UpdateAudio();


//-------------------------------------------------------------------------------------
// 	Declared in audio.cpp
//-------------------------------------------------------------------------------------
extern XAudio2_BufferNotify XAudio2_Notifier; 


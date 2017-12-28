#include "player.h"
#include "audio.h"

//-------------------------------------------------------------------------------------
// Audio data ..
//-------------------------------------------------------------------------------------	
AVPacket   g_AudioPacket;

//-------------------------------------------------------------------------------------
// Xaudio2 things ...
//-------------------------------------------------------------------------------------	
IXAudio2* g_pXAudio2 = NULL;
IXAudio2MasteringVoice* g_pMasteringVoice = NULL;



IXAudio2SourceVoice* g_pSourceVoice = NULL;
XAUDIO2_BUFFER g_SoundBuffer;
WAVEFORMATEX * g_pWfx  = NULL;
BYTE *  AudioBytes = NULL;
//XAudio2 event notifier
XAudio2_BufferNotify XAudio2_Notifier; 




HRESULT InitAudio(FFMpegData * pFFMpegData){
	if(pFFMpegData->pAudioCodecCtx==NULL){
		OutputDebugStringA( "pAudioCodecCtx = NULL\n");
		return E_FAIL;
	}

	//-------------------------------------------------------------------------------------
	// Initialise Audio
	//-------------------------------------------------------------------------------------	
	HRESULT hr;
	if( FAILED( hr = XAudio2Create( &g_pXAudio2, 0, XboxThread5 ) ) )
	{
		OutputDebugStringA( "Failed to init XAudio2 engine\n");
		return E_FAIL;
	}

	//-------------------------------------------------------------------------------------
	// Create a mastering voice
	//-------------------------------------------------------------------------------------	
	if( FAILED( hr = g_pXAudio2->CreateMasteringVoice( &g_pMasteringVoice ) ) )
	{
		OutputDebugStringA( "Failed creating mastering voice\n");
		return E_FAIL;
	}
	//-------------------------------------------------------------------------------------
	// Create source voice
	//-------------------------------------------------------------------------------------	
	WAVEFORMATEXTENSIBLE wfx;
	memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));

	wfx.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE ;
	wfx.Format.nSamplesPerSec       = pFFMpegData->pAudioCodecCtx->sample_rate;//48000 by default
	wfx.Format.nChannels            = pFFMpegData->pAudioCodecCtx->channels;
	wfx.Format.wBitsPerSample       = 16;
	wfx.Format.nBlockAlign          = wfx.Format.nChannels*16/8;
	wfx.Format.nAvgBytesPerSec      = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
	wfx.Format.cbSize               = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;
	if(wfx.Format.nChannels == 1){
		wfx.dwChannelMask = SPEAKER_MONO;
	}
	else if(wfx.Format.nChannels == 2){
		wfx.dwChannelMask = SPEAKER_STEREO;
	}
	else if(wfx.Format.nChannels == 5){
		wfx.dwChannelMask = SPEAKER_5POINT1;
	}
	wfx.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

	//Create Audio buffer (freeed by callback)
	AudioBytes = (BYTE *)malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE*sizeof(BYTE));//new BYTE[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	//AudioBytes = (BYTE *)XPhysicalAlloc(AVCODEC_MAX_AUDIO_FRAME_SIZE*sizeof(BYTE),MAXULONG_PTR,2048,PAGE_READWRITE | PAGE_WRITECOMBINE );

	//-------------------------------------------------------------------------------------
	//	Source voice
	//-------------------------------------------------------------------------------------
	unsigned int flags = 0;//XAUDIO2_VOICE_NOSRC | XAUDIO2_VOICE_NOPITCH;
	if(FAILED( g_pXAudio2->CreateSourceVoice(&g_pSourceVoice,(WAVEFORMATEX*)&wfx, flags , 1.0f, &XAudio2_Notifier)	))
	{
		OutputDebugStringA("CreateSourceVoice failed\n");
		return E_FAIL;
	}

	//-------------------------------------------------------------------------------------
	// Start sound
	//-------------------------------------------------------------------------------------	
	if ( FAILED(g_pSourceVoice->Start( 0 ) ) )
	{
		OutputDebugStringA("g_pSourceVoice failed\n");
		return E_FAIL;
	}

	return S_OK;
}


HRESULT UpdateAudio(){
	return S_OK;
}
#include "player.h"
#include "audio.h"

//From Audio.cpp
extern BYTE *  AudioBytes;
extern XAUDIO2_BUFFER g_SoundBuffer;
extern IXAudio2SourceVoice* g_pSourceVoice;
extern XAudio2_BufferNotify XAudio2_Notifier;

HRESULT AddSoundToQueue(FFMpegData * data){

	int done = AVCODEC_MAX_AUDIO_FRAME_SIZE;

	if (avcodec_decode_audio2(
		data->pAudioCodecCtx,
		(int16_t *)AudioBytes,
		&done,
		data->packet.data,data->packet.size
		) < 0 ) 
	{
		OutputDebugString("Error in decoding audio\n");
		av_free_packet(&data->packet);
		//continue;
		return E_FAIL;
	}
	if (done) {
		BYTE * snd = (BYTE *)malloc( done * sizeof(BYTE));

		XMemCpy(snd,
			AudioBytes, 
			done * sizeof(BYTE)
		);
		
		XMemSet(&g_SoundBuffer,0,sizeof(XAUDIO2_BUFFER));

		g_SoundBuffer.AudioBytes = done;
		g_SoundBuffer.pAudioData = snd;
		g_SoundBuffer.pContext = (VOID*)snd;

		XAUDIO2_VOICE_STATE state;
		while( g_pSourceVoice->GetState( &state ), state.BuffersQueued > 60 )
		{
			WaitForSingleObject( XAudio2_Notifier.hBufferEndEvent, INFINITE );
		}

		g_pSourceVoice->SubmitSourceBuffer( &g_SoundBuffer );

	}

	return S_OK;
}

#include "DVDAudioDevice.h"
#include "..\..\utils\Log.h"

CDVDAudio::CDVDAudio()
{
	m_bInitialized = false;

	m_pXAudio2 = NULL;
	m_pMasteringVoice = NULL;
	m_pSourceVoice = NULL;
}

CDVDAudio::~CDVDAudio()
{
}

bool CDVDAudio::Create(int iChannels, int iBitrate, int iBitsPerSample, bool bPasstrough)
{
    UINT32 flags = 0;
    XAudio2Create( &m_pXAudio2, flags );

	m_pXAudio2->CreateMasteringVoice( &m_pMasteringVoice );

	//Create source voice
	WAVEFORMATEXTENSIBLE wfx;
	memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));

	wfx.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE ;
	wfx.Format.nSamplesPerSec       = iBitrate;
	wfx.Format.nChannels            = iChannels;
	wfx.Format.wBitsPerSample       = 16;
	wfx.Format.nBlockAlign          = wfx.Format.nChannels*16/8;
	wfx.Format.nAvgBytesPerSec      = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
	wfx.Format.cbSize               = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;

	if(wfx.Format.nChannels == 1)
		wfx.dwChannelMask = SPEAKER_MONO;
	else if(wfx.Format.nChannels == 2)
		wfx.dwChannelMask = SPEAKER_STEREO;
	else if(wfx.Format.nChannels == 5)
		wfx.dwChannelMask = SPEAKER_5POINT1;

	wfx.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

	//Source voice
	m_pXAudio2->CreateSourceVoice(&m_pSourceVoice,(WAVEFORMATEX*)&wfx, flags , 1.0f, NULL);

	//Start sound
	m_pSourceVoice->Start( 0 );

	m_bInitialized = true;

	return true;
}

void CDVDAudio::Destroy()
{
	while(m_quBuffers.size() != 0 )
	{
		BYTE * pSnd = m_quBuffers.front();
		m_quBuffers.pop();
		
		if(pSnd)
			free(pSnd);
	}

	if(m_pSourceVoice)
	m_pSourceVoice->DestroyVoice();

	if(m_pMasteringVoice)
	m_pMasteringVoice->DestroyVoice();

	if(m_pXAudio2)
	m_pXAudio2->Release();
	
	m_bInitialized = false;
}

DWORD CDVDAudio::AddPackets(unsigned char* data, DWORD len)
{
	if(!m_bInitialized)
		return 0;

	BYTE * snd = (BYTE *)malloc( len * sizeof(BYTE));

	memcpy(snd, data, len * sizeof(BYTE) );
		
	memset(&m_SoundBuffer,0,sizeof(XAUDIO2_BUFFER));
	
	m_SoundBuffer.AudioBytes = len;
	m_SoundBuffer.pAudioData = snd;
	m_SoundBuffer.pContext = (VOID*)snd;

	XAUDIO2_VOICE_STATE state;
	
	while( m_pSourceVoice->GetState( &state ), state.BuffersQueued >= 32 )
		Sleep(1);
	
	m_pSourceVoice->SubmitSourceBuffer( &m_SoundBuffer );

	m_quBuffers.push(snd);

	//HACK HACK! - Marty - I'm not sure how to do this with properly
	//					   with XAudio2, this works atm tho..
	if(m_quBuffers.size() >= 36)
	{
		while(m_quBuffers.size() >= 36 )
		{
			BYTE * pSnd = m_quBuffers.front();
			m_quBuffers.pop();
		
			free(pSnd);
			pSnd = NULL;
		}
	}

	return 0;
}

void CDVDAudio::Flush()
{
/*	while(m_quBuffers.size() != 0 )
	{
		BYTE * pSnd = m_quBuffers.front();
		m_quBuffers.pop();
		
		if(pSnd)
			free(pSnd);
	}
*/
	if(m_bInitialized)
	{
		m_pSourceVoice->Stop();
		m_pSourceVoice->Start();
	}
}

float CDVDAudio::GetDelay()
{
	return 0.008f;
}
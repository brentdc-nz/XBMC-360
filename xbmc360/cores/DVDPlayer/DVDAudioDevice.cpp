#include "DVDAudioDevice.h"
#include "..\..\utils\Log.h"
#include "..\..\guilib\AudioContext.h"
#include "DVDClock.h"

#define MAX_BUFFERS 64

CDVDAudio::CDVDAudio() : m_hBufferEndEvent( CreateEvent( NULL, FALSE, FALSE, NULL ) )
{
	InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0x00000400);

	m_bInitialized = false;

	m_pXAudio2 = NULL;
	m_pSourceVoice = NULL;

	m_iBufferSize = 0;

	m_iBitrate = 0;
	m_iChannels = 0;
}

CDVDAudio::~CDVDAudio()
{
	CloseHandle( m_hBufferEndEvent );
	DeleteCriticalSection(&m_CriticalSection);
}

bool CDVDAudio::Create(int iChannels, int iBitrate, int iBitsPerSample, bool bPasstrough)
{
	m_pXAudio2 = g_audioContext.GetXAudio2Device();

	if(!m_pXAudio2)
		return false;

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

	wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	m_iBitrate = iBitrate;
	m_iChannels = iChannels;

	// Source voice
	m_pXAudio2->CreateSourceVoice(&m_pSourceVoice,(WAVEFORMATEX*)&wfx, NULL, 1.0f, this);

	// Start sound
	m_pSourceVoice->Start( 0 );

	m_bInitialized = true;

	return true;
}

void CDVDAudio::Destroy()
{
	// Flushing the buffers will stop any leaks,
	// OnBufferEnd() is called for any queued buffers
	m_pSourceVoice->FlushSourceBuffers();

	if(m_pSourceVoice)
		m_pSourceVoice->DestroyVoice();

	m_bInitialized = false;
}

DWORD CDVDAudio::AddPackets(unsigned char* data, DWORD len)
{
	if(!m_bInitialized)
		return 0;

	EnterCriticalSection(&m_CriticalSection);

	XAUDIO2_BUFFER SoundBuffer;

	SSoundData* pSoundData = NULL;
	pSoundData = new SSoundData;

	BYTE* pSnd = (BYTE*)malloc(len * sizeof(BYTE));
	memcpy(pSnd, data, len * sizeof(BYTE));
		
	memset(&SoundBuffer,0,sizeof(XAUDIO2_BUFFER));

	pSoundData->iSize = len;
	pSoundData->pVoid = (VOID*)pSnd;
	
	SoundBuffer.AudioBytes = len;
	SoundBuffer.pAudioData = pSnd;
	SoundBuffer.pContext = (VOID*)pSoundData;

	XAUDIO2_VOICE_STATE state;
	
	while(m_pSourceVoice->GetState( &state ), state.BuffersQueued >= MAX_BUFFERS)
		Sleep(1);
	
	m_pSourceVoice->SubmitSourceBuffer(&SoundBuffer);
	
	m_iBufferSize += pSoundData->iSize;

	LeaveCriticalSection(&m_CriticalSection);

	return 0;
}

void CDVDAudio::OnBufferEnd(void * pBufferContext)
{
	EnterCriticalSection(&m_CriticalSection);

	SSoundData* pSoundData = NULL;
	pSoundData = (SSoundData*)pBufferContext;

	m_iBufferSize -= pSoundData->iSize;

	if(pSoundData->pVoid)
	{
		free(pSoundData->pVoid);
		pSoundData->pVoid = NULL;
	}

	if(pSoundData)
	{
		delete pSoundData;
		pSoundData = NULL;
	}

	LeaveCriticalSection(&m_CriticalSection);
}

void CDVDAudio::Flush()
{
	if(m_bInitialized)
		m_pSourceVoice->FlushSourceBuffers();
}

void CDVDAudio::Pause()
{
	if(m_bInitialized)
		m_pSourceVoice->Stop();
}

void CDVDAudio::Resume()
{
	if(m_bInitialized)
		m_pSourceVoice->Start();
}

int CDVDAudio::GetBytesInBuffer()
{
	int iSize = 0;

	if(m_bInitialized)
	{
		EnterCriticalSection(&m_CriticalSection);

		iSize = m_iBufferSize;
	
		LeaveCriticalSection(&m_CriticalSection);	
	}

	return iSize;
}

float CDVDAudio::GetDelay()
{
	__int64 delay;

	if (m_iChannels != 0 && m_iBitrate != 0)
	{
		delay = (__int64)(0.008f * DVD_TIME_BASE);
  		delay += ((__int64)GetBytesInBuffer() * DVD_TIME_BASE) / (m_iBitrate * m_iChannels * 2);

		return (float)delay;
	}

	return 0LL;
}
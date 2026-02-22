#define NOMINMAX
#include "XAudio2.h"
#include "Application.h"
#include "utils\Log.h"
#include "utils\TimeUtils.h"
#include "guilib\AudioContext.h"
#include "Settings.h"

#define BUFFER CHUNKLEN * 20
#define CHUNKLEN 512

#define MAX_BUFFERS 64

struct SSoundData
{
	int iSize;
	void* pVoid;
	int iStream;  // Stream index for buffer-played callback
};

// XAudio2 Callbacks
void CXAudio2::OnBufferStart(void * pBufferContext)
{
	EnterCriticalSection(&m_CriticalSection);

	SSoundData* pSoundData = NULL;
	pSoundData = (SSoundData*)pBufferContext;

	if (/*dwStatus == XMEDIAPACKET_STATUS_SUCCESS &&*/ m_VisBuffer)
	{
		if (m_VisBytes + pSoundData->iSize <= m_VisMaxBytes)
		{
			memcpy(m_VisBuffer + m_VisBytes, pSoundData->pVoid, pSoundData->iSize);
			m_VisBytes += pSoundData->iSize;
		}
	}

	LeaveCriticalSection(&m_CriticalSection);
}

void CXAudio2::OnBufferEnd(void * pBufferContext)
{
	EnterCriticalSection(&m_CriticalSection);

	SSoundData* pSoundData = NULL;
	pSoundData = (SSoundData*)pBufferContext;

	// Fire the buffer-played callback so PAPlayer can track actual playback position
	if (m_bufferPlayedCallback && pSoundData)
	{
		m_bufferPlayedCallback(m_bufferPlayedContext, pSoundData->iSize, pSoundData->iStream);
	}

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

//***********************************************************************************************

CXAudio2::CXAudio2(int iChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, const char* strAudioCodec/* = ""*/, bool bIsMusic/* = false*/)
: m_hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
{
	CLog::Log(LOGERROR,"Creating XAudio2 Audio Renderer..");

	if(iChannels == 0)
		iChannels = 2;

	m_pXAudio2 = NULL;
	m_pSourceVoice = NULL;

	InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0x00000400);
	m_bInitialized = false;

	m_pXAudio2 = g_audioContext.GetXAudio2Device();
  
	m_uiSamplesPerSec = uiSamplesPerSec;
	m_uiBitsPerSample = uiBitsPerSample;

	ZeroMemory(&m_wfxex, sizeof(m_wfxex));

	m_wfxex.Format.nChannels       = iChannels;
	m_wfxex.Format.nSamplesPerSec  = uiSamplesPerSec;
	m_wfxex.Format.wBitsPerSample  = uiBitsPerSample;
	m_wfxex.Format.nBlockAlign     = uiBitsPerSample / 8 * iChannels;
	m_wfxex.Format.nAvgBytesPerSec = m_wfxex.Format.nBlockAlign * m_wfxex.Format.nSamplesPerSec;
	m_wfxex.Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
	m_wfxex.Format.cbSize          = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX) ;
	m_wfxex.SubFormat              = KSDATAFORMAT_SUBTYPE_PCM;  
	m_wfxex.Samples.wValidBitsPerSample = uiBitsPerSample;

	DWORD dwMask[] = 
	{
		SPEAKER_FRONT_CENTER,
		SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT,
		SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER,
		SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT,
		SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT,
		SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT
	};

	if(iChannels > 0 && iChannels < 7)
		m_wfxex.dwChannelMask = dwMask[iChannels-1];
	else
		m_wfxex.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;  
  
	if(m_pXAudio2->CreateSourceVoice(&m_pSourceVoice,(WAVEFORMATEX*)&m_wfxex, NULL, 1.0f, this) != S_OK)
		return;

	m_nCurrentVolume = g_settings.m_nVolumeLevel;

	m_pSourceVoice->Stop();
	m_pSourceVoice->SetVolume(g_audioContext.MilliBelsToVolume(m_nCurrentVolume));

	m_bInitialized = true;

	m_timePerPacket = 1.0f / (float)(iChannels*(uiBitsPerSample/8) * uiSamplesPerSec);
	m_packetsSent = 0;
	m_bPaused = false;
	m_lastUpdate = CTimeUtils::GetTimeMS();

	m_VisBuffer = (PBYTE)malloc(m_VisMaxBytes = iChannels * m_uiSamplesPerSec * (m_uiBitsPerSample / 8) / 20);
	m_VisBytes = 0;

	m_bufferPlayedCallback = NULL;
	m_bufferPlayedContext = NULL;
	m_streamIndex = 0;
}

//***********************************************************************************************

CXAudio2::~CXAudio2()
{
	if(m_pSourceVoice)
		Deinitialize();
}

//***********************************************************************************************

HRESULT CXAudio2::Deinitialize()
{
	if(m_pSourceVoice)
	{
		m_pSourceVoice->Stop();
		m_pSourceVoice->FlushSourceBuffers();

		// Wait for all OnBufferEnd callbacks to complete before destroying.
		// FlushSourceBuffers is async on Xbox 360 - callbacks fire on the
		// audio thread and must finish their free() calls before we
		// destroy the voice and delete the critical section they use.
		XAUDIO2_VOICE_STATE state;
		do {
			m_pSourceVoice->GetState(&state);
		} while (state.BuffersQueued > 0);

		m_pSourceVoice->DestroyVoice();
	}

	if (m_VisBuffer)
		free(m_VisBuffer);
	
	m_VisBuffer = NULL;

	m_pXAudio2 = NULL;
	m_pSourceVoice = NULL;
	m_bInitialized = false;

	DeleteCriticalSection(&m_CriticalSection);

	return S_OK;
}

//***********************************************************************************************

void CXAudio2::Flush()
{
	m_lastUpdate = CTimeUtils::GetTimeMS();
	m_packetsSent = 0;

	Pause();
}

//***********************************************************************************************

HRESULT CXAudio2::Pause()
{
	if(m_bPaused) return S_OK;
	if(!m_pSourceVoice) return S_FALSE;

	m_bPaused = true;

	// We flush on pause
	m_pSourceVoice->FlushSourceBuffers();
	m_pSourceVoice->Stop();

	return S_OK;
}

//***********************************************************************************************

HRESULT CXAudio2::Resume()
{
	if(!m_bPaused) return S_OK;
	if(!m_pSourceVoice) return S_FALSE;

	m_bPaused = false;
	m_pSourceVoice->Start();

	return S_OK;
}

//***********************************************************************************************

HRESULT CXAudio2::Stop()
{
	if(!m_pSourceVoice) return S_FALSE;

	m_bPaused = true;
	m_pSourceVoice->Stop();
	m_pSourceVoice->FlushSourceBuffers();

	m_lastUpdate = CTimeUtils::GetTimeMS();
	m_packetsSent = 0;
	m_VisBytes = 0;

	return S_OK;
}

//***********************************************************************************************

long CXAudio2::GetCurrentVolume() const
{
	return m_nCurrentVolume;
}

//***********************************************************************************************

void CXAudio2::Mute(bool bMute)
{
	if(bMute)
		m_pSourceVoice->SetVolume(0);
	else
		m_pSourceVoice->SetVolume(g_audioContext.MilliBelsToVolume(m_nCurrentVolume));
}

//***********************************************************************************************

HRESULT CXAudio2::SetCurrentVolume(long nVolume)
{
	m_nCurrentVolume = nVolume;
	return m_pSourceVoice->SetVolume(g_audioContext.MilliBelsToVolume(m_nCurrentVolume));
}

//***********************************************************************************************

DWORD CXAudio2::GetSpace()
{
	Update();

	if(!m_bInitialized || !m_pSourceVoice)
		return 0;

	XAUDIO2_VOICE_STATE state;
	m_pSourceVoice->GetState(&state);

	return MAX_BUFFERS - state.BuffersQueued;
}

//***********************************************************************************************

DWORD CXAudio2::AddPackets(unsigned char* data, DWORD len)
{
	if(m_bPaused || GetSpace() == 0 || len == 0)
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
	pSoundData->iStream = m_streamIndex;
	
	SoundBuffer.AudioBytes = len;
	SoundBuffer.pAudioData = pSnd;
	SoundBuffer.Flags = 0;  // Do NOT set XAUDIO2_END_OF_STREAM on every buffer
	SoundBuffer.pContext = (VOID*)pSoundData;

	m_pSourceVoice->SubmitSourceBuffer(&SoundBuffer);
	
	if(!m_bPaused/* && !(status & DSBSTATUS_PLAYING)*/)
		m_pSourceVoice->Start(0);

	LeaveCriticalSection(&m_CriticalSection);

	int add = (len / GetChunkLen()) * GetChunkLen();
	m_packetsSent += add;

	return add;
}

//***********************************************************************************************

float CXAudio2::GetDelay()
{
	Update();

	return m_timePerPacket * (float)m_packetsSent;
}

//***********************************************************************************************

float CXAudio2::GetCacheTime()
{
	if(!m_bInitialized || !m_pXAudio2)
		return 0.0;

	XAUDIO2_PERFORMANCE_DATA perfData;
	m_pXAudio2->GetPerformanceData(&perfData);

	return perfData.CurrentLatencyInSamples / (float)m_uiSamplesPerSec;
}

//***********************************************************************************************

DWORD CXAudio2::GetChunkLen()
{
	return (int)CHUNKLEN;
}

//***********************************************************************************************

int CXAudio2::SetPlaySpeed(int iSpeed)
{
	return 0;
}

//***********************************************************************************************

DWORD CXAudio2::GetVisData(BYTE* pDest, DWORD maxLen)
{
	EnterCriticalSection(&m_CriticalSection);

	DWORD len = 0;
	
	if (m_VisBytes > 0 && pDest)
	{
		len = (m_VisBytes < maxLen) ? m_VisBytes : maxLen;
		memcpy(pDest, m_VisBuffer, len);
		m_VisBytes = 0;
	}
	
	LeaveCriticalSection(&m_CriticalSection);
	
	return len;
}

//***********************************************************************************************

void CXAudio2::WaitCompletion()
{
	while(m_packetsSent > 0)
		Update();
}

//***********************************************************************************************

void CXAudio2::SwitchChannels(int iAudioStream, bool bAudioOnAllSpeakers)
{
    return;
}

//***********************************************************************************************

void CXAudio2::SetBufferPlayedCallback(BufferPlayedCallback callback, void* pCallerContext, int stream)
{
	m_bufferPlayedCallback = callback;
	m_bufferPlayedContext = pCallerContext;
	m_streamIndex = stream;
}

//***********************************************************************************************

void CXAudio2::Update()
{
	long currentTime = CTimeUtils::GetTimeMS();
	long deltaTime = (currentTime - m_lastUpdate);

	if(m_bPaused)
	{
		m_lastUpdate += deltaTime;
		return;
	}

	double d = (double)deltaTime / 1000.0f;

	if (currentTime != m_lastUpdate)
	{
		double i = (d / (double)m_timePerPacket);
		m_packetsSent -= (long)i;

		if(m_packetsSent < 0)
			m_packetsSent = 0;
		
		m_lastUpdate = currentTime;
	}
}
#include "NullSound.h"
#include "Application.h"
#include "utils\Log.h"
#include "utils\TimeUtils.h"

#define BUFFER CHUNKLEN * 20
#define CHUNKLEN 512

//***********************************************************************************************

CNullSound::CNullSound(IAudioCallback* pCallback, int iChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, const char* strAudioCodec/* = ""*/, bool bIsMusic/* = false*/)
{
	CLog::Log(LOGERROR,"Creating a Null Audio Renderer, Check your audio settings as this should not happen");
	
	if(iChannels == 0)
		iChannels = 2;
/*
	bool bAudioOnAllSpeakers(false);
	g_audioContext.SetupSpeakerConfig(iChannels, bAudioOnAllSpeakers, bIsMusic);
	g_audioContext.SetActiveDevice(CAudioContext::DIRECTSOUND_DEVICE);
*/
	m_timePerPacket = 1.0f / (float)(iChannels*(uiBitsPerSample/8) * uiSamplesPerSec);
	m_packetsSent = 0;
	m_paused = 0;
	m_lastUpdate = CTimeUtils::GetTimeMS();
}

//***********************************************************************************************

CNullSound::~CNullSound()
{
	Deinitialize();
}

//***********************************************************************************************

HRESULT CNullSound::Deinitialize()
{
//	g_audioContext.SetActiveDevice(CAudioContext::DEFAULT_DEVICE);
	return true;
}

//***********************************************************************************************

void CNullSound::Flush()
{
	m_lastUpdate = CTimeUtils::GetTimeMS();
	m_packetsSent = 0;
	Pause();
}

//***********************************************************************************************

HRESULT CNullSound::Pause()
{
	m_paused = true;
	return true;
}

//***********************************************************************************************

HRESULT CNullSound::Resume()
{
	m_paused = false;
	return true;
}

//***********************************************************************************************

HRESULT CNullSound::Stop()
{
	Flush();
	return true;
}

//***********************************************************************************************

long CNullSound::GetCurrentVolume() const
{
	return m_nCurrentVolume;
}

//***********************************************************************************************

void CNullSound::Mute(bool bMute)
{
}

//***********************************************************************************************

HRESULT CNullSound::SetCurrentVolume(long nVolume)
{
	m_nCurrentVolume = nVolume;
	return true;
}

//***********************************************************************************************

DWORD CNullSound::GetSpace()
{
	Update();

	if(BUFFER > m_packetsSent)
		return (int)BUFFER - m_packetsSent;
	else
		return 0;
}

//***********************************************************************************************

DWORD CNullSound::AddPackets(unsigned char* data, DWORD len)
{
	if(m_paused || GetSpace() == 0)
		return 0;

	int add = ( len / GetChunkLen() ) * GetChunkLen();
	m_packetsSent += add;

	return add;
}

//***********************************************************************************************

float CNullSound::GetDelay()
{
	Update();

	return m_timePerPacket * (float)m_packetsSent;
}

//***********************************************************************************************

float CNullSound::GetCacheTime()
{
	return GetDelay();
}

//***********************************************************************************************

DWORD CNullSound::GetChunkLen()
{
	return (int)CHUNKLEN;
}

//***********************************************************************************************

int CNullSound::SetPlaySpeed(int iSpeed)
{
	return 0;
}

//***********************************************************************************************

void CNullSound::RegisterAudioCallback(IAudioCallback *pCallback)
{
}

//***********************************************************************************************

void CNullSound::UnRegisterAudioCallback()
{
}

//***********************************************************************************************

void CNullSound::WaitCompletion()
{
	while(m_packetsSent > 0)
		Update();
}

//***********************************************************************************************

void CNullSound::SwitchChannels(int iAudioStream, bool bAudioOnAllSpeakers)
{
    return;
}

//***********************************************************************************************

void CNullSound::Update()
{
	long currentTime = CTimeUtils::GetTimeMS();
	long deltaTime = (currentTime - m_lastUpdate);

	if(m_paused)
	{
		m_lastUpdate += deltaTime;
		return;
	}

	double d = (double)deltaTime / 1000.0f;

	if(currentTime != m_lastUpdate)
	{
		double i = (d / (double)m_timePerPacket);
		m_packetsSent -= (long)i;
		
		if(m_packetsSent < 0)
			m_packetsSent = 0;

		m_lastUpdate = currentTime;
	}
}
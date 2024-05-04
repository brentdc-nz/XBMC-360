#include "DVDAudio.h"

#ifdef _XBOX
#include "..\AudioRenderers\NullSound.h"
#include "..\AudioRenderers\XAudio2.h"
#endif

#include "DVDClock.h"
#include "utils\Log.h"
#include "DVDPlayerAudio.h"

using namespace std;

CDVDAudio::CDVDAudio(volatile bool &bStop)
 : m_bStop(bStop)
{
	m_pAudioDecoder = NULL;
	m_pCallback = NULL;
	m_iBufferSize = 0;
	m_dwPacketSize = 0;
	m_pBuffer = NULL;
	m_bPassthrough = false;
	m_iBitsPerSample = 0;
	m_iBitrate = 0;
	m_iChannels = 0;
}

CDVDAudio::~CDVDAudio()
{
	CSingleLock lock (m_critSection);
	if(m_pAudioDecoder)
	{
		m_pAudioDecoder->Deinitialize();
		delete m_pAudioDecoder;
	}
	free(m_pBuffer);
}

bool CDVDAudio::Create(const DVDAudioFrame &audioframe, CodecID codec)
{
	CLog::Log(LOGNOTICE, "Creating audio device with codec id: %i, channels: %i, sample rate: %i, %s", codec, audioframe.channels, audioframe.sample_rate, audioframe.passthrough ? "pass-through" : "no pass-through");

	// If passthrough isset do something else
	CSingleLock lock (m_critSection);

	const char* codecstring = "";

	if(codec == CODEC_ID_AAC)
#ifdef USE_LIBFAAD
		codecstring = "AAC";
#else
		codecstring = "FF-AAC"; // ffmpeg decodes aac with a different channel order so we need to differentiate
#endif
	else if(codec == CODEC_ID_VORBIS)
		codecstring = "Vorbis";
	else if(codec == CODEC_ID_AC3 || codec == CODEC_ID_DTS)
		codecstring = ""; // TODO, fix ac3 and dts decoder to output standard windows mapping
	else
		codecstring = "PCM";

//	m_pAudioDecoder = new CNullSound(m_pCallback, audioframe.channels, audioframe.sample_rate, audioframe.bits_per_sample, codecstring);
	m_pAudioDecoder = new CXAudio2(m_pCallback, audioframe.channels, audioframe.sample_rate, audioframe.bits_per_sample, codecstring);

#if 0
#ifdef _XBOX
	// We don't allow resampling now, there is a bug in sscc that causes it to return the wrong chunklen.
	if(audioframe.passthrough)
		m_pAudioDecoder = new CAc97DirectSound(m_pCallback, audioframe.channels, audioframe.sample_rate, audioframe.bits_per_sample, true); // true = resample, 128 buffers
	else
		m_pAudioDecoder = new CASyncDirectSound(m_pCallback, audioframe.channels, audioframe.sample_rate, audioframe.bits_per_sample, codecstring);
#else
	if(audioframe.passthrough)
		return false;

	m_pAudioDecoder = new CWin32DirectSound(m_pCallback, audioframe.channels, audioframe.sample_rate, audioframe.bits_per_sample, false, codecstring);
#endif
#endif

	if(!m_pAudioDecoder) return false;

	m_iChannels = audioframe.channels;
	m_iBitrate = audioframe.sample_rate;
	m_iBitsPerSample = audioframe.bits_per_sample;
	m_bPassthrough = audioframe.passthrough;

	m_dwPacketSize = m_pAudioDecoder->GetChunkLen();
	
	if(m_bPaused)
		m_pAudioDecoder->Pause();

	m_iBufferSize = 0;

	return true;
}

bool CDVDAudio::IsValidFormat(const DVDAudioFrame &audioframe)
{
	if(!m_pAudioDecoder)
		return false;

	if(audioframe.passthrough != m_bPassthrough)
		return false;

	if(audioframe.channels != m_iChannels
	  || audioframe.sample_rate != m_iBitrate
	  || audioframe.bits_per_sample != m_iBitsPerSample)
		return false;

	return true;
}

double CDVDAudio::GetDelay()
{
	CSingleLock lock (m_critSection);

	double delay = 0.0;
	if(m_pAudioDecoder)
		delay = m_pAudioDecoder->GetDelay();

	DWORD bps = m_iChannels * m_iBitrate * (m_iBitsPerSample>>3);
	if(m_iBufferSize && bps)
		delay += (double)m_iBufferSize / bps;

	return delay * DVD_TIME_BASE;
}

double CDVDAudio::GetCacheTime()
{
	CSingleLock lock (m_critSection);

	if(!m_pAudioDecoder)
		return 0.0;

	return m_pAudioDecoder->GetCacheTime();
}

DWORD CDVDAudio::AddPackets(const DVDAudioFrame &audioframe)
{
	CSingleLock lock(m_critSection);

	unsigned char* data = audioframe.data;
	DWORD len = audioframe.size;

	DWORD total = len;
	DWORD copied;

	// When paused, we need to buffer all data as renderers don't need to accept it
	if(m_bPaused)
	{
		m_pBuffer = (BYTE*)realloc(m_pBuffer, m_iBufferSize + len);
		memcpy(m_pBuffer+m_iBufferSize, data, len);
		m_iBufferSize += len;
		return len;
	}

	if(m_iBufferSize > 0) // See if there are carryover bytes from the last call. need to add them 1st.
	{
		copied = /*std::*/min(m_dwPacketSize - m_iBufferSize % m_dwPacketSize, len); // Smaller of either the data provided or the leftover data
		if(copied)
		{
			m_pBuffer = (BYTE*)realloc(m_pBuffer, m_iBufferSize + copied);
			memcpy(m_pBuffer + m_iBufferSize, data, copied); // Tack the caller's data onto the end of the buffer
			data += copied; // Move forward in caller's data
			len -= copied; // Decrease amount of data available from caller
			m_iBufferSize += copied; // Increase amount of data available in buffer
		}

		if(m_iBufferSize < m_dwPacketSize) // If we don't have enough data to give to the renderer, wait until next time
			return copied;

		if(AddPacketsRenderer(m_pBuffer, m_iBufferSize, lock) != m_iBufferSize)
		{
			m_iBufferSize = 0;
			CLog::Log(LOGERROR, "%s - failed to add leftover bytes to render", __FUNCTION__);
			return copied;
		}

		m_iBufferSize = 0;

		if(!len)
			return copied; // We used up all the caller's data
	}

	copied = AddPacketsRenderer(data, len, lock);
	data += copied;
	len -= copied;

	// If we have more data left, save it for the next call to this funtion
	if (len > 0 && !m_bStop)
	{
		m_pBuffer     = (BYTE*)realloc(m_pBuffer, len);
		m_iBufferSize = len;
		memcpy(m_pBuffer, data, len);
	}

	return total;
}

void CDVDAudio::SetVolume(int iVolume)
{
	CSingleLock lock (m_critSection);
	if(m_pAudioDecoder) m_pAudioDecoder->SetCurrentVolume(iVolume);
}

void CDVDAudio::Destroy()
{
	CSingleLock lock (m_critSection);

	if(m_pAudioDecoder)
	{
		m_pAudioDecoder->Stop();
		m_pAudioDecoder->Deinitialize();
		delete m_pAudioDecoder;
	}

	free(m_pBuffer);
	m_pBuffer = NULL;
	m_dwPacketSize = 0;
	m_pAudioDecoder = NULL;
	m_iBufferSize = 0;
	m_iChannels = 0;
	m_iBitrate = 0;
	m_iBitsPerSample = 0;
	m_bPassthrough = false;
}

void CDVDAudio::Flush()
{
	CSingleLock lock (m_critSection);

	if(m_pAudioDecoder)
	{
		m_pAudioDecoder->Stop();
		m_pAudioDecoder->Resume();
	}
	m_iBufferSize = 0;
}

void CDVDAudio::Finish()
{
	CSingleLock lock (m_critSection);
	if(!m_pAudioDecoder)
		return;

	DWORD silence = m_dwPacketSize - m_iBufferSize % m_dwPacketSize;

	if(silence > 0 && m_iBufferSize > 0)
	{
		CLog::Log(LOGDEBUG, "CDVDAudio::Drain - adding %d bytes of silence, buffer size: %d, chunk size: %d", silence, m_iBufferSize, m_dwPacketSize);
		m_pBuffer = (BYTE*)realloc(m_pBuffer, m_dwPacketSize);
		memset(m_pBuffer+m_iBufferSize, 0, silence);
		m_iBufferSize += silence;
	}

	if(AddPacketsRenderer(m_pBuffer, m_iBufferSize, lock) != m_iBufferSize)
		CLog::Log(LOGERROR, "CDVDAudio::Drain - failed to play the final %d bytes", m_iBufferSize);

	m_iBufferSize = 0;
}

void CDVDAudio::Drain()
{
	Finish();
	CSingleLock lock (m_critSection);

	if(m_pAudioDecoder)
		m_pAudioDecoder->WaitCompletion();
}

void CDVDAudio::Pause()
{
	CSingleLock lock (m_critSection);
	m_bPaused = true;
	if(m_pAudioDecoder) m_pAudioDecoder->Pause();
}

void CDVDAudio::Resume()
{
	CSingleLock lock (m_critSection);
	m_bPaused = false;
	if(m_pAudioDecoder) m_pAudioDecoder->Resume();
}

DWORD CDVDAudio::AddPacketsRenderer(unsigned char* data, DWORD len, CSingleLock &lock)
{
	if(!m_pAudioDecoder)
		return 0;

	DWORD bps = m_iChannels * m_iBitrate * (m_iBitsPerSample>>3);
	if(!bps)
		return 0;

	// Calculate a timeout when this definitely should be done
	double timeout;
	timeout  = DVD_SEC_TO_TIME(m_pAudioDecoder->GetDelay() + (double)len / bps);
	timeout += DVD_SEC_TO_TIME(1.0);
	timeout += CDVDClock::GetAbsoluteClock();

	DWORD  total = len;
	DWORD  copied;
	do
	{
		copied = m_pAudioDecoder->AddPackets(data, len);
		data += copied;
		len -= copied;
		if (len < m_dwPacketSize)
			break;

		if(copied == 0 && timeout < CDVDClock::GetAbsoluteClock())
		{
			CLog::Log(LOGERROR, "CDVDAudio::AddPacketsRenderer - timeout adding data to renderer");
			break;
		}

		lock.Leave();
		Sleep(1);
		lock.Enter();
	} while (!m_bStop);

	return total - len;
}
#include "utils/log.h"
#include "paplayer.h"
#include "CodecFactory.h"
#include "guilib\GUIInfoManager.h"
#include "guilib\AudioContext.h"
//#include "music\tags\MusicInfoTag.h"
#include "Settings.h"
#include "AdvancedSettings.h"
#include "FileItem.h"
#include "utils\TimeUtils.h"
#include "cores\AudioRenderers\XAudio2.h"

#define TIME_TO_CACHE_NEXT_FILE 5000L    // 5 seconds
#define TIME_TO_CROSS_FADE      10000L   // 10 seconds
#define VOLUME_FFWD_MUTE        900      // 9dB - mute amount during fast-forward/rewind

// Static callback: fired by XAudio2 when a buffer has actually been played out.
// This mirrors the original Xbox DirectSound StreamCallback, ensuring m_bytesSentOut
// tracks real playback position rather than submission position.
static void StaticBufferPlayedCallback(void* pCallerContext, DWORD bytesPlayed, int stream)
{
	PAPlayer* pPlayer = (PAPlayer*)pCallerContext;
	pPlayer->OnBufferPlayed(bytesPlayed, stream);
}

// PAP: Psycho-acoustic Audio Player
// Supporting all open audio codec standards.
// First one being nullsoft's nsv audio decoder format

PAPlayer::PAPlayer(IPlayerCallback& callback) : IPlayer(callback)
{
	m_bIsPlaying = false;
	m_bPaused = false;
	m_cachingNextFile = false;
	m_currentlyCrossFading = false;
	m_bQueueFailed = false;

	m_currentDecoder = 0;

	m_iSpeed = 1;
	m_SeekTime = -1;
	m_IsFFwdRewding = false;
	m_timeOffset = 0;

	m_currentStream = 0;

	m_pAudioRenderer[0] = NULL;
	m_pAudioRenderer[1] = NULL;

	m_bytesSentOut = 0;

	m_BytesPerSecond = 0;
	m_SampleRate = 0;
	m_Channels = 0;
	m_BitsPerSample = 0;

	m_SampleRateOutput = 0;
	m_BitsPerSampleOutput = 0;

	m_forceFadeToNext = false;
	m_CacheLevel = 0;
	m_LastCacheLevelCheck = 0;

	m_visBufferLength = 0;
	m_pCallback = NULL;

	m_currentFile = new CFileItem;
	m_nextFile = new CFileItem;
}

PAPlayer::~PAPlayer()
{
	CloseFileInternal(true);
	delete m_currentFile;
	delete m_nextFile;
}

bool PAPlayer::OpenFile(const CFileItem& file, const CPlayerOptions &options)
{
	if (m_currentlyCrossFading) CloseFileInternal(false); // User seems to be in a hurry

	m_crossFading = 0;//g_guiSettings.GetInt("musicplayer.crossfade"); // TODO BRENT - Disabled for now

	// No crossfading for cdda, cd-reading goes mad and no crossfading for last.fm doesn't like two connections
//	if (file.IsCDDA() || file.IsLastFM()) m_crossFading = 0;  // TODO BRENT

	if (m_crossFading && IsPlaying())
	{
		// Do a short crossfade on trackskip
		// set to max 2 seconds for these prev/next transitions
		if (m_crossFading > 2) m_crossFading = 2;

		// Queue for crossfading
		bool result = QueueNextFile(file, false);
		
		if (result)
		{
			// Crossfading value may be update by QueueNextFile when nr of channels changed
			if (!m_crossFading) // Swap to next track
				m_decoder[m_currentDecoder].SetStatus(STATUS_ENDED);
			else // Force to fade to next track immediately
				m_forceFadeToNext = true;
		}
		return result;
	}

	// Normal opening of file, nothing playing or crossfading not enabled
	// however no need to return to gui audio device
	CloseFileInternal(false);

	// Always open the file using the current decoder
	m_currentDecoder = 0;

	if (!m_decoder[m_currentDecoder].Create(file, (__int64)(options.starttime * 1000), m_crossFading))
		return false;

	m_iSpeed = 1;
	m_bPaused = false;
	m_bStopPlaying = false;
	m_bytesSentOut = 0;

	CLog::Log(LOGINFO, "PAPlayer: Playing %s", file.GetPath().c_str());

	m_timeOffset = (__int64)(options.starttime * 1000);

	m_decoder[m_currentDecoder].GetDataFormat(&m_Channels, &m_SampleRate, &m_BitsPerSample);

	CStdString codecname;
	ICodec* codec = m_decoder[m_currentDecoder].GetCodec();
	
	if(codec)
		codecname = codec->m_CodecName;

	SetupXAudio2(m_Channels);

	if (!CreateStream(m_currentStream, m_Channels, m_SampleRate, m_BitsPerSample, codecname))
	{
		m_decoder[m_currentDecoder].Destroy();
		CLog::Log(LOGERROR, "PAPlayer::Unable to create audio stream");
		return false;
	}

	*m_currentFile = file;

	if (ThreadHandle() == NULL)
		Create();

	m_startEvent.Set();

	m_bIsPlaying = true;
	m_cachingNextFile = false;
	m_currentlyCrossFading = false;
	m_forceFadeToNext = false;
	m_bQueueFailed = false;

	m_decoder[m_currentDecoder].Start(); // Start playback

	// Resume XAudio2 renderer to begin outputting audio
	if (m_pAudioRenderer[m_currentStream])
		m_pAudioRenderer[m_currentStream]->Resume();

	return true;
}

bool PAPlayer::CloseFileInternal(bool bAudioDevice /*= true*/)
{
	if (IsPaused())
		Pause();

	m_bStopPlaying = true;
	m_bStop = true;

	StopThread();

	// Kill both our streams if we need to
	for (int i = 0; i < 2; i++)
	{
		m_decoder[i].Destroy();
		FreeStream(i);
	}

	m_currentFile->Reset();
	m_nextFile->Reset();

	// Note: Original Xbox used g_audioContext.SetActiveDevice(CAudioContext::DEFAULT_DEVICE)
	// to release DirectSound when playback stops. XAudio2 stays initialized globally,
	// so no equivalent call is needed here.

	return true;
}

bool PAPlayer::QueueNextFile(const CFileItem &file)
{
	return QueueNextFile(file, true);
}

bool PAPlayer::QueueNextFile(const CFileItem &file, bool checkCrossFading)
{
	if (IsPaused())
		Pause();

	if (file.GetPath() == m_currentFile->GetPath() &&
		file.m_lStartOffset > 0 && 
		file.m_lStartOffset == m_currentFile->m_lEndOffset)
	{
		// Continuing on a .cue sheet item - return true to say we'll handle the transistion
		*m_nextFile = file;
		return true;
	}

	// Check if we can handle this file at all
	int decoder = 1 - m_currentDecoder;
	__int64 seekOffset = (file.m_lStartOffset * 1000) / 75;
	
	if (!m_decoder[decoder].Create(file, seekOffset, m_crossFading))
	{
		m_bQueueFailed = true;
		return false;
	}
	
	// Ok, we're good to go on queuing this one up
	CLog::Log(LOGINFO, "PAPlayer: Queuing next file %s", file.GetPath().c_str());

	m_bQueueFailed = false;
	
	if (checkCrossFading)
		UpdateCrossFadingTime(file);

	unsigned int channels, samplerate, bitspersample;
	m_decoder[decoder].GetDataFormat(&channels, &samplerate, &bitspersample);

	CStdString codecname;
	ICodec* codec = m_decoder[m_currentDecoder].GetCodec();
	
	if(codec)
		codecname = codec->m_CodecName;

	// Check the number of channels isn't changing (else we can't do crossfading)
	if (m_crossFading && m_decoder[m_currentDecoder].GetChannels() == channels && channels <= 2)
	{
		// Crossfading - need to create a new stream
		if (!CreateStream(1 - m_currentStream, channels, samplerate, bitspersample, codecname))
		{
			m_decoder[decoder].Destroy();
			CLog::Log(LOGERROR, "PAPlayer::Unable to create audio stream");
		}
	}
	else
	{ 
		// No crossfading if nr of channels is not the same
		m_crossFading = 0;
	}

	*m_nextFile = file;

	return true;
}

void PAPlayer::OnNothingToQueueNotify()
{
	// Nothing to queue, stop playing
	m_bQueueFailed = true;
}

void PAPlayer::Seek(bool bPlus, bool bLargeStep)
{
	__int64 seek;

	if (g_advancedSettings.m_musicUseTimeSeeking && GetTotalTime() > 2*g_advancedSettings.m_musicTimeSeekForwardBig)
	{
		if (bLargeStep)
			seek = bPlus ? g_advancedSettings.m_musicTimeSeekForwardBig : g_advancedSettings.m_musicTimeSeekBackwardBig;
		else
			seek = bPlus ? g_advancedSettings.m_musicTimeSeekForward : g_advancedSettings.m_musicTimeSeekBackward;

		seek *= 1000;
		seek += GetTime();
	}
	else
	{
		float percent;
		
		if (bLargeStep)
			percent = bPlus ? (float)g_advancedSettings.m_musicPercentSeekForwardBig : (float)g_advancedSettings.m_musicPercentSeekBackwardBig;
		else
			percent = bPlus ? (float)g_advancedSettings.m_musicPercentSeekForward : (float)g_advancedSettings.m_musicPercentSeekBackward;
		seek = (__int64)(GetTotalTime64()*(GetPercentage()+percent)/100);
	}

	SeekTime(seek);
}

void PAPlayer::SeekPercentage(float fPercent /*=0*/)
{
	if (fPercent < 0.0f) fPercent = 0.0f;
	if (fPercent > 100.0f) fPercent = 100.0f;
	SeekTime((__int64)(fPercent * 0.01f * (float)GetTotalTime64()));
}

float PAPlayer::GetPercentage()
{
	return (float)GetTime() * 100.0f / GetTotalTime64();
}

void PAPlayer::SetDynamicRangeCompression(long drc)
{
	// TODO: Add volume amplification
}

bool PAPlayer::CanSeek()
{
	return ((m_decoder[m_currentDecoder].TotalTime() > 0) && m_decoder[m_currentDecoder].CanSeek());
}

void PAPlayer::ToFFRW(int iSpeed)
{
	m_iSpeed = iSpeed;
	m_callback.OnPlayBackSpeedChanged(iSpeed);

	if (iSpeed == 1)
	{
		m_callback.OnPlayBackResumed();
		CLog::Log(LOGDEBUG, "PAPlayer: Playback resumed");
	}
}

int PAPlayer::GetCacheLevel() const
{
	return m_CacheLevel;
}

void PAPlayer::Pause()
{
	if (!m_bIsPlaying) return;

	m_bPaused = !m_bPaused;
	
	if (m_bPaused)
	{
		// Pause both streams if we're crossfading
		if (m_pAudioRenderer[m_currentStream]) m_pAudioRenderer[m_currentStream]->Pause();
		if (m_currentlyCrossFading && m_pAudioRenderer[1 - m_currentStream])
			m_pAudioRenderer[1 - m_currentStream]->Pause();

		m_callback.OnPlayBackPaused();
		CLog::Log(LOGDEBUG, "PAPlayer: Playback paused");
	}
	else
	{
		if (m_pAudioRenderer[m_currentStream]) m_pAudioRenderer[m_currentStream]->Resume();
		if (m_currentlyCrossFading && m_pAudioRenderer[1 - m_currentStream])
			m_pAudioRenderer[1 - m_currentStream]->Resume();

		m_callback.OnPlayBackResumed();
		CLog::Log(LOGDEBUG, "PAPlayer: Playback resumed");
	}
}

void PAPlayer::SetupXAudio2(int channels)
{
	// Ensure the global XAudio2 audio context is initialized
	g_audioContext.Initialize();
}

bool PAPlayer::CreateStream(int num, int channels, int samplerate, int bitspersample, CStdString codecname)
{
	// Free any existing stream first
	FreeStream(num);

	// Output as 16-bit PCM to XAudio2 (decoder gives us float, we convert in AddPacketsToStream)
	unsigned int outputBitsPerSample = 16;
	unsigned int outputSampleRate = samplerate;

	m_SampleRateOutput = outputSampleRate;
	m_BitsPerSampleOutput = outputBitsPerSample;
	m_BytesPerSecond = channels * (outputBitsPerSample / 8) * outputSampleRate;

	CLog::Log(LOGINFO, "PAPlayer::CreateStream - Creating XAudio2 stream %d: %d channels, %d Hz, %d bps, codec: %s",
		num, channels, outputSampleRate, outputBitsPerSample, codecname.c_str());

	// Create the XAudio2 renderer (same renderer used by DVDPlayer)
	m_pAudioRenderer[num] = new CXAudio2(channels, outputSampleRate, outputBitsPerSample,
		codecname.c_str(), true /* bIsMusic */);

	if (!m_pAudioRenderer[num])
	{
		CLog::Log(LOGERROR, "PAPlayer::CreateStream - Failed to create XAudio2 renderer for stream %d", num);
		return false;
	}

	// Set initial volume
	m_pAudioRenderer[num]->SetCurrentVolume(g_settings.m_nVolumeLevel);

	// Register the buffer-played callback so m_bytesSentOut tracks actual playback
	// position (mirrors original Xbox DirectSound StreamCallback behavior)
	((CXAudio2*)m_pAudioRenderer[num])->SetBufferPlayedCallback(StaticBufferPlayedCallback, this, num);

	// Fire off our init to our visualization callback
	if (m_pCallback)
		m_pCallback->OnInitialize(channels, m_SampleRateOutput, m_BitsPerSampleOutput);

	return true;
}

void PAPlayer::FreeStream(int stream)
{
	if (m_pAudioRenderer[stream])
	{
		// Deinitialize handles Stop + FlushSourceBuffers + DestroyVoice + DeleteCriticalSection
		// No need to call Stop() separately (it would double-flush)
		m_pAudioRenderer[stream]->Deinitialize();
		delete m_pAudioRenderer[stream];
		m_pAudioRenderer[stream] = NULL;
	}
}

bool PAPlayer::AddPacketsToStream(int stream, CAudioDecoder &dec)
{
	if (!m_pAudioRenderer[stream] || dec.GetStatus() == STATUS_NO_FILE)
		return false;

	bool ret = false;

	DWORD chunkLen = m_pAudioRenderer[stream]->GetChunkLen();
	unsigned int channels = dec.GetChannels();
	if (channels == 0) channels = m_Channels;
	if (channels == 0) return false;

	// Minimum samples per submission: must produce at least chunkLen bytes
	unsigned int minSamples = chunkLen / sizeof(short);

	// Feed data while the renderer has buffer slots and decoder has data
	while (m_pAudioRenderer[stream]->GetSpace() > 0)
	{
		unsigned int dataSize = dec.GetDataSize();
		if (dataSize < minSamples)
			break;

		unsigned int samplesToRead = std::min<unsigned int>(dataSize, OUTPUT_SAMPLES);
		samplesToRead -= (samplesToRead % channels);
		if (samplesToRead < minSamples)
			break;

		float *floatData = (float *)dec.GetData(samplesToRead);
		if (!floatData)
			break;

		for (unsigned int i = 0; i < samplesToRead; i++)
		{
			float sample = floatData[i];
			if (sample > 1.0f) sample = 1.0f;
			if (sample < -1.0f) sample = -1.0f;
			m_pcm16Buffer[i] = (short)(sample * 32767.0f);
		}

		DWORD bytesToSend = samplesToRead * sizeof(short);
		DWORD bytesSent = m_pAudioRenderer[stream]->AddPackets((unsigned char *)m_pcm16Buffer, bytesToSend);

		if (bytesSent > 0)
		{
			// Don't count bytes here - they are counted in OnBufferPlayed callback
			// when XAudio2 has actually played the buffer out (not just buffered it).
			// This ensures GetTime() tracks real playback position.
			ret = true;
		}
		else
		{
			break;
		}
	}

	return ret;
}

void PAPlayer::FlushStreams()
{
	for (int stream = 0; stream < 2; stream++)
	{
		if (m_pAudioRenderer[stream])
		{
			m_pAudioRenderer[stream]->Flush();
			// CXAudio2::Flush() internally pauses the renderer via Pause().
			// Resume so AddPackets can accept new data after the flush.
			m_pAudioRenderer[stream]->Resume();
		}
	}
}

// XAudio2 port: Sets volume on the current stream (replaces DirectSound SetVolume)
void PAPlayer::SetVolume(long nVolume)
{
	SetStreamVolume(m_currentStream, nVolume);
}

void PAPlayer::SetStreamVolume(int stream, long nVolume)
{
	// Clamp to valid millibel range (XAudio2 port of DSBVOLUME_MIN/MAX clamping)
	if (nVolume > VOLUME_MAXIMUM) nVolume = VOLUME_MAXIMUM;
	if (nVolume < VOLUME_MINIMUM) nVolume = VOLUME_MINIMUM;
	if (m_pAudioRenderer[stream])
		m_pAudioRenderer[stream]->SetCurrentVolume(nVolume);
}

void PAPlayer::UpdateCrossFadingTime(const CFileItem& file)
{
/*	if (m_crossFading = g_guiSettings.GetInt("musicplayer.crossfade")) // TODO BRENT
	{
		if (m_crossFading && ( file.IsCDDA() || file.IsLastFM() ||
			(
			!g_guiSettings.GetBool("musicplayer.crossfadealbumtracks") && m_currentFile->HasMusicInfoTag() && file.HasMusicInfoTag() &&
			(m_currentFile->GetMusicInfoTag()->GetAlbum() != "") &&
			(m_currentFile->GetMusicInfoTag()->GetAlbum() == file.GetMusicInfoTag()->GetAlbum()) &&
			(m_currentFile->GetMusicInfoTag()->GetDiscNumber() == file.GetMusicInfoTag()->GetDiscNumber()) &&
			(m_currentFile->GetMusicInfoTag()->GetTrackNumber() == file.GetMusicInfoTag()->GetTrackNumber() - 1)
			)
			)
		)
		{
			m_crossFading = 0;
		}
	}*/
}

void PAPlayer::WaitForStream()
{
	if (m_pAudioRenderer[m_currentStream])
		m_pAudioRenderer[m_currentStream]->WaitCompletion();
}

void PAPlayer::Process()
{
	CLog::Log(LOGDEBUG, "PAPlayer: Thread started");

	if (m_startEvent.WaitMSec(100))
	{
		m_startEvent.Reset();

		m_callback.OnPlayBackStarted();

		do
		{
			if (!m_bPaused)
			{
				if (!ProcessPAP())
					break;
			}
			else
			{
				Sleep(100);
			}
		}
		while (!m_bStopPlaying && m_bIsPlaying && !m_bStop);

		CLog::Log(LOGINFO, "PAPlayer: End of playback reached");
		m_bIsPlaying = false;
		
		if (!m_bStopPlaying && !m_bStop)
			m_callback.OnPlayBackEnded();
		else
			m_callback.OnPlayBackStopped();
	}
}

bool PAPlayer::ProcessPAP()
{
	/*
	* Here's what we should be doing in each player loop:
	*
	* 1.  Run DoWork() on our audio device to actually output audio.
	*
	* 2.  Pass our current buffer to the audio device to see if it wants anything,
	*     and if so, reduce our buffer size accordingly.
	*
	* 3.  Check whether we have space in our buffer for more data, and if so,
	*     read some more in.
	*
	* 4.  Check for end of file and return false if we reach it.
	*
	* 5.  Perform any seeking and ffwd/rewding as necessary.
	*
	* 6.  If we don't do anything in 2...5, we can take a breather and break out for sleeping.
	*/

	while (true)
	{
		if (m_bStop) return false;

		// Check for .cue sheet item end
		if (m_currentFile->m_lEndOffset && GetTime() >= GetTotalTime64())
		{
			CLog::Log(LOGINFO, "PAPlayer: Passed end of track in a .cue sheet item");
			m_decoder[m_currentDecoder].SetStatus(STATUS_ENDED);
		}

		// Check whether we need to send off our callbacks etc.
		int status = m_decoder[m_currentDecoder].GetStatus();
		
		if (status == STATUS_NO_FILE)
			return false;

		UpdateCacheLevel();

		// Check whether we should queue the next file up
		if ((GetTotalTime64() > 0) && GetTotalTime64() - GetTime() < TIME_TO_CACHE_NEXT_FILE + m_crossFading * 1000L && !m_cachingNextFile)
		{
			// Request the next file from our application
			m_callback.OnQueueNextItem();
			m_cachingNextFile = true;
		}

		if (m_crossFading && m_decoder[0].GetChannels() == m_decoder[1].GetChannels() && m_decoder[0].GetChannels() <= 2)
		{
			if (((GetTotalTime64() - GetTime() < m_crossFading * 1000L) || (m_forceFadeToNext)) && !m_currentlyCrossFading)
			{
				// Request the next file from our application
				if (m_decoder[1 - m_currentDecoder].GetStatus() == STATUS_QUEUED && m_pAudioRenderer[1 - m_currentStream])
				{
					m_currentlyCrossFading = true;
					
					if (m_forceFadeToNext)
					{
						m_forceFadeToNext = false;
						m_crossFadeLength = m_crossFading * 1000L;
					}
					else
					{
						m_crossFadeLength = GetTotalTime64() - GetTime();
					}
					
					m_currentDecoder = 1 - m_currentDecoder;
					m_decoder[m_currentDecoder].Start();
					m_currentStream = 1 - m_currentStream;
					CLog::Log(LOGDEBUG, "Starting Crossfade - resuming stream %i", m_currentStream);

					if (m_pAudioRenderer[m_currentStream])
						m_pAudioRenderer[m_currentStream]->Resume();
					m_callback.OnPlayBackStarted();
					m_timeOffset = m_nextFile->m_lStartOffset * 1000 / 75;
					m_bytesSentOut = 0;
					*m_currentFile = *m_nextFile;
					m_nextFile->Reset();
					m_cachingNextFile = false;
				}
			}
		}

		// Check for EOF and queue the next track if applicable
		if (m_decoder[m_currentDecoder].GetStatus() == STATUS_ENDED)
		{
			// Time to swap tracks
			if (m_nextFile->GetPath() != m_currentFile->GetPath() ||
				!m_nextFile->m_lStartOffset ||
				m_nextFile->m_lStartOffset != m_currentFile->m_lEndOffset)
			{
				// Don't have a .cue sheet item
				int nextstatus = m_decoder[1 - m_currentDecoder].GetStatus();
			
				if (nextstatus == STATUS_QUEUED || nextstatus == STATUS_QUEUING || nextstatus == STATUS_PLAYING)
				{
					// Swap streams
					CLog::Log(LOGDEBUG, "PAPlayer: Swapping tracks %i to %i", m_currentDecoder, 1-m_currentDecoder);
				
					if (!m_crossFading || m_decoder[0].GetChannels() != m_decoder[1].GetChannels() || m_decoder[0].GetChannels() > 2)
					{
						// Playing gapless (we use only the 1 output stream in this case)
						int prefixAmount = m_decoder[m_currentDecoder].GetDataSize();
						CLog::Log(LOGDEBUG, "PAPlayer::Prefixing %i samples of old data to new track for gapless playback", prefixAmount);
						m_decoder[1 - m_currentDecoder].PrefixData(m_decoder[m_currentDecoder].GetData(prefixAmount), prefixAmount);
					
						// Check if we need to change the resampler (due to format change)
						unsigned int channels, samplerate, bitspersample;
						m_decoder[m_currentDecoder].GetDataFormat(&channels, &samplerate, &bitspersample);
						unsigned int channels2, samplerate2, bitspersample2;
						m_decoder[1 - m_currentDecoder].GetDataFormat(&channels2, &samplerate2, &bitspersample2);

						CStdString codecname;
						ICodec* codec = m_decoder[1 - m_currentDecoder].GetCodec();

						if(codec)
							codecname = codec->m_CodecName;

						// Change of channels - Reinitialize our speaker configuration
						if (channels != channels2 || channels > 2)
						{
							CLog::Log(LOGWARNING, "PAPlayer: Channel number has changed - restarting XAudio2 stream");
							FreeStream(m_currentStream);
							SetupXAudio2(channels2);
           
							if (!CreateStream(m_currentStream, channels2, samplerate2, bitspersample2, codecname))
							{
								CLog::Log(LOGERROR, "PAPlayer: Error creating stream!");
								return false;
							}
							if (m_pAudioRenderer[m_currentStream])
								m_pAudioRenderer[m_currentStream]->Resume();
						}
						else if (samplerate != samplerate2 || bitspersample != bitspersample2)
						{
							// Sample rate or bit depth changed - recreate the XAudio2 stream
							CLog::Log(LOGINFO, "PAPlayer: Format change detected - recreating XAudio2 stream");
							FreeStream(m_currentStream);
							if (!CreateStream(m_currentStream, channels2, samplerate2, bitspersample2, codecname))
							{
								CLog::Log(LOGERROR, "PAPlayer: Error creating stream after format change!");
								return false;
							}
							if (m_pAudioRenderer[m_currentStream])
								m_pAudioRenderer[m_currentStream]->Resume();
						}
						CLog::Log(LOGINFO, "PAPlayer: Starting new track");

						m_decoder[m_currentDecoder].Destroy();
						m_decoder[1 - m_currentDecoder].Start();
						m_callback.OnPlayBackStarted();
						m_timeOffset = m_nextFile->m_lStartOffset * 1000 / 75;
						m_bytesSentOut = 0;
						*m_currentFile = *m_nextFile;
						m_nextFile->Reset();
						m_cachingNextFile = false;
						m_currentDecoder = 1 - m_currentDecoder;
					}
					else
					{
						// Cross fading - Shouldn't ever get here - if we do, return false
						if (!m_currentlyCrossFading)
						{
							CLog::Log(LOGERROR, "End of file Reached before crossfading kicked in!");
							return false;
						}
						else
						{
							CLog::Log(LOGINFO, "End of file reached before crossfading finished!");
							return false;
						}
					}
				}
				else
				{
					if (GetTotalTime64() <= 0 && !m_bQueueFailed)
					{
						// We did not know the duration so didn't queue the next song, try queueing it now
						if (!m_cachingNextFile)
						{
							// Request the next file from our application
							m_callback.OnQueueNextItem();
							m_cachingNextFile = true;
						}
					}
					else
					{
						// No track queued - Return and get another one once we are finished
						// with the current stream
						WaitForStream();
						return false;
					}
				}
			}
			else
			{
				// Set the next track playing (.cue sheet)
				m_decoder[m_currentDecoder].SetStatus(STATUS_PLAYING);
				m_callback.OnPlayBackStarted();
				m_timeOffset = m_nextFile->m_lStartOffset * 1000 / 75;
				m_bytesSentOut = 0;
				*m_currentFile = *m_nextFile;
				m_nextFile->Reset();
				m_cachingNextFile = false;
			}
		}

		// Handle seeking and ffwd/rewding.
		HandleSeeking();
		
		if (!HandleFFwdRewd())
		{
			// Need to skip to the next track - let's see if we already have another one
			m_decoder[m_currentDecoder].SetStatus(STATUS_ENDED);
			continue; // loop around to start the next track
		}

		// Let our decoding stream(s) do their thing
		DWORD time = CTimeUtils::timeGetTime();
		int retVal = m_decoder[m_currentDecoder].ReadSamples(PACKET_SIZE);
		
		if (retVal == RET_ERROR)
		{
			m_decoder[m_currentDecoder].Destroy();
			return false;
		}
		
		int retVal2 = m_decoder[1 - m_currentDecoder].ReadSamples(PACKET_SIZE);

		if (retVal2 == RET_ERROR)
		{
			m_decoder[1 - m_currentDecoder].Destroy();
		}
		DWORD time2 = CTimeUtils::timeGetTime();

		// If we're cross-fading, then we do this for both streams, otherwise
		// we do it just for the one stream.
		if (m_currentlyCrossFading)
		{
			if (GetTime() >= m_crossFadeLength)  // Finished
			{
				CLog::Log(LOGDEBUG, "Finished Crossfading");
				m_currentlyCrossFading = false;
				SetStreamVolume(m_currentStream, g_settings.m_nVolumeLevel);
				FreeStream(1 - m_currentStream);
				m_decoder[1 - m_currentDecoder].Destroy();
			}
			else
			{
				float fraction = (float)(m_crossFadeLength - GetTime()) / (float)m_crossFadeLength - 0.5f;
				
				// Make sure we can take valid logs.
				if (fraction > 0.499f) fraction = 0.499f;
				if (fraction < -0.499f) fraction = -0.499f;

				float volumeCurrent = 2000.0f * log10(0.5f - fraction);
				float volumeNext = 2000.0f * log10(0.5f + fraction);

				SetStreamVolume(m_currentStream, g_settings.m_nVolumeLevel + (int)volumeCurrent);
				SetStreamVolume(1 - m_currentStream, g_settings.m_nVolumeLevel + (int)volumeNext);

				if (AddPacketsToStream(1 - m_currentStream, m_decoder[1 - m_currentDecoder]))
					retVal2 = RET_SUCCESS;
			}
		}

		// Add packets as necessary
		if (AddPacketsToStream(m_currentStream, m_decoder[m_currentDecoder]))
			retVal = RET_SUCCESS;

		if (retVal == RET_SLEEP && retVal2 == RET_SLEEP)
			Sleep(1);
		
		DWORD time3 = CTimeUtils::timeGetTime();
		//CLog::Log(LOGINFO, "Time Decoding: %i, Time Resampling: %i, bytes processed %i, buffer 1 state %i, buffer 2 state %i", time2-time, time3-time2, dataToRead, m_decoder[m_currentDecoder].GetDataSize(), m_decoder[1 - m_currentDecoder].GetDataSize());
	}

	return true;
}

void PAPlayer::HandleSeeking()
{
	if (m_SeekTime != -1)
	{
		DWORD time = CTimeUtils::timeGetTime();
		m_timeOffset = m_decoder[m_currentDecoder].Seek(m_SeekTime);
		CLog::Log(LOGDEBUG, "Seek to time %f took %i ms", 0.001f * m_SeekTime, CTimeUtils::timeGetTime() - time);
		FlushStreams();
		m_bytesSentOut = 0;
		m_SeekTime = -1;
	}
	g_infoManager.m_performingSeek = false;
}

bool PAPlayer::HandleFFwdRewd()
{
	if (!m_IsFFwdRewding && m_iSpeed == 1)
		return true;  // Nothing to do

	if (m_IsFFwdRewding && m_iSpeed == 1)
	{ 
		// Stop ffwd/rewd
		m_IsFFwdRewding = false;
		SetStreamVolume(m_currentStream, g_settings.m_nVolumeLevel);
		m_bytesSentOut = 0;
		FlushStreams();

		return true;
	}

	// We're definitely fastforwarding or rewinding
	int snippet = m_BytesPerSecond / 2;
	
	if ( m_bytesSentOut >= snippet ) 
	{
		// Calculate offset to seek if we do FF/RW
		__int64 time = GetTime();
		if (m_IsFFwdRewding) snippet = (int)m_bytesSentOut;
		time += (__int64)((double)snippet * (m_iSpeed - 1.0) / m_BytesPerSecond * 1000.0);

		// Is our offset inside the track range?
		
		if (time >= 0 && time <= m_decoder[m_currentDecoder].TotalTime())
		{
			// Just set next position to read
			m_IsFFwdRewding = true;  
			time += m_currentFile->m_lStartOffset * 1000 / 75;
			m_timeOffset = m_decoder[m_currentDecoder].Seek(time);
			m_bytesSentOut = 0;
			FlushStreams();
			SetStreamVolume(m_currentStream, g_settings.m_nVolumeLevel - VOLUME_FFWD_MUTE); // Mute during FFWD/REW
		}
		else if (time < 0)
		{
			// ...disable seeking and start the track again
			time = m_currentFile->m_lStartOffset * 1000 / 75;
			m_timeOffset = m_decoder[m_currentDecoder].Seek(time);
			m_bytesSentOut = 0;
			FlushStreams();
			m_iSpeed = 1;
			SetStreamVolume(m_currentStream, g_settings.m_nVolumeLevel);
		}
		// Is our next position greater then the end sector...
		else //if (time > m_codec->m_TotalTime)
		{
			// Restore volume level so the next track isn't muted
			SetStreamVolume(m_currentStream, g_settings.m_nVolumeLevel);
			CLog::Log(LOGDEBUG, "PAPlayer: End of track reached while seeking");
			return false;
		}
	}
	return true;
}

__int64 PAPlayer::GetTotalTime64()
{
	__int64 total = m_decoder[m_currentDecoder].TotalTime();

	if (m_currentFile->m_lEndOffset)
		total = m_currentFile->m_lEndOffset * 1000 / 75;

	if (m_currentFile->m_lStartOffset)
		total -= m_currentFile->m_lStartOffset * 1000 / 75;

	return total;
}

int PAPlayer::GetTotalTime()
{
	return (int)(GetTotalTime64()/1000);
}

__int64 PAPlayer::GetTime()
{
	__int64 timeplus = m_BytesPerSecond ? (__int64)(((float)m_bytesSentOut / (float)m_BytesPerSecond) * 1000.0) : 0;
	return m_timeOffset + timeplus - m_currentFile->m_lStartOffset * 1000 / 75;
}

void PAPlayer::UpdateCacheLevel()
{
	// Check cachelevel every .5 seconds
	if (m_LastCacheLevelCheck + 500 < GetTickCount())
	{
		ICodec* codec = m_decoder[m_currentDecoder].GetCodec();
		if (codec)
		{
			m_CacheLevel = codec->GetCacheLevel();
			m_LastCacheLevelCheck = GetTickCount();
			//CLog::Log(LOGDEBUG,"Cachelevel: %i%%", m_CacheLevel);
		}
	}
}

int PAPlayer::GetChannels()
{
  ICodec* codec = m_decoder[m_currentDecoder].GetCodec();
  if (codec)
    return codec->m_Channels;
  return 0;
}

int PAPlayer::GetAudioBitrate()
{
	ICodec* codec = m_decoder[m_currentDecoder].GetCodec();

	if (codec)
		return codec->m_Bitrate;

	return 0;
}

int PAPlayer::GetBitsPerSample()
{
	ICodec* codec = m_decoder[m_currentDecoder].GetCodec();
	
	if (codec)
		return codec->m_BitsPerSample;
	
	return 0;
}

int PAPlayer::GetSampleRate()
{
	ICodec* codec = m_decoder[m_currentDecoder].GetCodec();

	if (codec)
		return (int)((codec->m_SampleRate / 1000) + 0.5);
	
	return 0;
}

CStdString PAPlayer::GetAudioCodecName()
{
	ICodec* codec = m_decoder[m_currentDecoder].GetCodec();

	if (codec)
		return codec->m_CodecName;

	return "";
}

void PAPlayer::ResetTime()
{
	m_bytesSentOut = 0;
}

void PAPlayer::SeekTime(__int64 iTime /*=0*/)
{
	if (!CanSeek()) return;
	int seekOffset = (int)(iTime - GetTime());

	if (m_currentFile->m_lStartOffset)
		iTime += m_currentFile->m_lStartOffset * 1000 / 75;

	m_SeekTime = iTime;
	m_callback.OnPlayBackSeek((int)m_SeekTime, seekOffset);
	CLog::Log(LOGDEBUG, "PAPlayer::Seeking to time %f", 0.001f * m_SeekTime);
}

bool PAPlayer::SkipNext()
{
	if (m_decoder[m_currentDecoder].GetCodec() && m_decoder[m_currentDecoder].GetCodec()->SkipNext())
	{
		return true;
	}
	return false;
}

bool PAPlayer::HandlesType(const CStdString &type)
{
	ICodec* codec = CodecFactory::CreateCodec(type);

	if (codec && codec->CanInit())
	{
		delete codec;   
		return true;
	}

	if (codec)
		delete codec;

	return false;
}

void PAPlayer::DoAudioWork()
{
	if (m_pCallback && m_pAudioRenderer[m_currentStream])
	{
		m_visBufferLength = m_pAudioRenderer[m_currentStream]->GetVisData(m_visBuffer, PACKET_SIZE);

		if (m_visBufferLength)
		{
			m_pCallback->OnAudioData(m_visBuffer, m_visBufferLength);
			m_visBufferLength = 0;
		}
	}
}

void PAPlayer::RegisterAudioCallback(IAudioCallback *pCallback)
{
	m_pCallback = pCallback;

	if (m_pCallback)
		m_pCallback->OnInitialize(m_Channels, m_SampleRateOutput, m_BitsPerSampleOutput);
}

void PAPlayer::UnRegisterAudioCallback()
{
	m_pCallback = NULL;
}

void PAPlayer::OnBufferPlayed(DWORD bytesPlayed, int stream)
{
	// Only count bytes from the current stream (during crossfade, ignore the fading-out stream)
	if (stream == m_currentStream)
		m_bytesSentOut += bytesPlayed;
}
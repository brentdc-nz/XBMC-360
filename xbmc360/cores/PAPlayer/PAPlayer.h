#pragma once

#include "cores\IPlayer.h"
#include "utils\Thread.h"
#include "AudioDecoder.h"
// CHANGED: Removed #include "cores/ssrc.h" - resampler not used, XAudio2 handles output natively

class CFileItem;
class IAudioRenderer;
class IAudioCallback;

class PAPlayer : public IPlayer, public CThread
{
public:
	PAPlayer(IPlayerCallback& callback);
	virtual ~PAPlayer();

	virtual bool OpenFile(const CFileItem& file, const CPlayerOptions &options);
	virtual bool QueueNextFile(const CFileItem &file);
	virtual void OnNothingToQueueNotify();
	virtual bool CloseFile()       { return CloseFileInternal(true); }
	virtual bool CloseFileInternal(bool bAudioDevice = true);
	virtual bool IsPlaying() const { return m_bIsPlaying; }
	virtual void Pause();
	virtual bool IsPaused() const { return m_bPaused; }
	virtual bool HasVideo() const { return false; }
	virtual bool HasAudio() const { return true; }
	virtual bool CanSeek();
	virtual void Seek(bool bPlus = true, bool bLargeStep = false);
	virtual void SeekPercentage(float fPercent = 0.0f); 
	virtual float GetPercentage();
	virtual void SetVolume(long nVolume);
	virtual void SetDynamicRangeCompression(long drc);
	virtual void GetAudioInfo( CStdString& strAudioInfo) {}
	virtual void GetVideoInfo( CStdString& strVideoInfo) {}
	virtual void GetGeneralInfo( CStdString& strVideoInfo) {}
	virtual void Update(bool bPauseDrawing = false) {}
	virtual void GetVideoRect(RECT& SrcRect, RECT& DestRect){}
	virtual void GetVideoAspectRatio(float& fAR) {}
	virtual void ToFFRW(int iSpeed = 0);
	virtual int GetCacheLevel() const;
	virtual int GetTotalTime();
	__int64 GetTotalTime64();
	virtual int GetAudioBitrate();
	virtual int GetChannels();
	virtual int GetBitsPerSample();
	virtual int GetSampleRate();
	virtual CStdString GetAudioCodecName();
	virtual __int64 GetTime();
	virtual void ResetTime();
	virtual void SeekTime(__int64 iTime = 0);

	// Skip to next track/item inside the current media (if supported)
	virtual bool SkipNext();

	virtual void RegisterAudioCallback(IAudioCallback *pCallback);
	virtual void UnRegisterAudioCallback();

	static bool HandlesType(const CStdString &type);
	virtual void DoAudioWork();

	// Called by XAudio2 OnBufferEnd callback when a buffer has actually been played out.
	// This mirrors the original Xbox DirectSound StreamCallback for accurate time tracking.
	void OnBufferPlayed(DWORD bytesPlayed, int stream);

protected:
	virtual void OnStartup() {}
	virtual void Process();
	virtual void OnExit() {};

	void HandleSeeking();
	bool HandleFFwdRewd();

	bool m_bPaused;
	bool m_bIsPlaying;
	bool m_bQueueFailed;
	bool m_bStopPlaying;
	bool m_cachingNextFile;
	int  m_crossFading;
	bool m_currentlyCrossFading;
	__int64 m_crossFadeLength;

	CEvent m_startEvent;

	int m_iSpeed; // Current playing speed

private:
	bool ProcessPAP(); // Does the actual reading and decode from our PAP

	__int64 m_SeekTime;
	int     m_IsFFwdRewding;
	__int64 m_timeOffset; 
	bool    m_forceFadeToNext;

	int m_currentDecoder;
	CAudioDecoder m_decoder[2]; // Our 2 audiodecoders (for crossfading + precaching)

	void SetupXAudio2(int channels);

	bool AddPacketsToStream(int stream, CAudioDecoder &dec);
	void FreeStream(int stream);
	bool CreateStream(int stream, int channels, int samplerate, int bitspersample, CStdString codec = "");
	void FlushStreams();
	void WaitForStream();
	void SetStreamVolume(int stream, long nVolume);
  
	void UpdateCrossFadingTime(const CFileItem& file);
	bool QueueNextFile(const CFileItem &file, bool checkCrossFading);
	void UpdateCacheLevel();

	int m_currentStream;

	IAudioRenderer* m_pAudioRenderer[2];

	// Temporary buffer for float-to-PCM16 conversion (XAudio2 receives PCM16 via AddPackets)
	short m_pcm16Buffer[OUTPUT_SAMPLES];

	__int64          m_bytesSentOut;

	// Format (this should be stored/retrieved from the audio device object probably)
	unsigned int     m_SampleRate;
	unsigned int     m_Channels;
	unsigned int     m_BitsPerSample;
	unsigned int     m_BytesPerSecond;

	unsigned int     m_SampleRateOutput;
	unsigned int     m_BitsPerSampleOutput;

	unsigned int     m_CacheLevel;
	unsigned int     m_LastCacheLevelCheck;

	// Our file
	CFileItem*        m_currentFile;
	CFileItem*        m_nextFile;

	// Stuff for visualisation
	BYTE m_visBuffer[PACKET_SIZE];
	unsigned int m_visBufferLength;
	IAudioCallback*  m_pCallback;
};
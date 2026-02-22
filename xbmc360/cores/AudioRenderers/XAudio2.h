#ifndef H_CXAUDIO2
#define H_CXAUDIO2

#include "IAudioRenderer.h"
#include <xtl.h>
#include <xaudio2.h>

// Callback fired when XAudio2 finishes playing a buffer.
// Parameters: pCallerContext (user pointer), bytesPlayed (size of the completed buffer), stream (stream index)
typedef void (*BufferPlayedCallback)(void* pCallerContext, DWORD bytesPlayed, int stream);

class CXAudio2 : public IAudioRenderer, public IXAudio2VoiceCallback
{
public:
	CXAudio2(int iChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, const char* strAudioCodec = "", bool bIsMusic = false);
	CXAudio2();

	virtual DWORD GetChunkLen();
	virtual float GetDelay();
	virtual float GetCacheTime();

	virtual ~CXAudio2();

	virtual DWORD AddPackets(unsigned char* data, DWORD len);

	virtual DWORD GetSpace();
	virtual HRESULT Deinitialize();
	virtual HRESULT Pause();
	virtual HRESULT Stop();
	virtual HRESULT Resume();

	virtual long GetCurrentVolume() const;
	virtual void Mute(bool bMute);
	virtual HRESULT SetCurrentVolume(LONG nVolume);
	virtual int SetPlaySpeed(int iSpeed);
	virtual void WaitCompletion();
	virtual void SwitchChannels(int iAudioStream, bool bAudioOnAllSpeakers);

	virtual void Flush();

	// Visualization data: pull accumulated PCM data for vis rendering
	virtual DWORD GetVisData(BYTE* pDest, DWORD maxLen);

	// Register a callback that fires when a buffer has been fully played out.
	// This allows callers (e.g. PAPlayer) to track actual playback position
	// rather than submission position.
	virtual void SetBufferPlayedCallback(BufferPlayedCallback callback, void* pCallerContext, int stream);

	// XAudio2 Callbacks
	void OnStreamEnd() { SetEvent( m_hBufferEndEvent ); }
	void OnVoiceProcessingPassEnd() {}
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
	void OnLoopEnd(void * pBufferContext) {}
	void OnVoiceError(void * pBufferContext, HRESULT Error) {}
	void OnBufferStart(void * pBufferContext);
	void OnBufferEnd(void * pBufferContext);

private:
	HANDLE m_hBufferEndEvent;
	CRITICAL_SECTION m_CriticalSection;
    IXAudio2* m_pXAudio2;
	IXAudio2SourceVoice* m_pSourceVoice;
	WAVEFORMATEXTENSIBLE m_wfxex;

	bool m_bInitialized;
	bool m_bPaused;

	unsigned int m_uiSamplesPerSec;
	unsigned int m_uiBitsPerSample;
	long m_nCurrentVolume;
	float m_timePerPacket;
	int m_packetsSent;
	long m_lastUpdate;

	void Update();

	PBYTE m_VisBuffer;
	DWORD m_VisBytes;
	DWORD m_VisMaxBytes;

	// Buffer-played callback (mirrors original Xbox DirectSound StreamCallback)
	BufferPlayedCallback m_bufferPlayedCallback;
	void* m_bufferPlayedContext;
	int m_streamIndex;
};

#endif //H_CXAUDIO2
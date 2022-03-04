#ifndef H_CXAUDIO2
#define H_CXAUDIO2

#include "IAudioRenderer.h"
#include "IAudioCallback.h"
#include <xtl.h>
#include <xaudio2.h>

extern void RegisterAudioCallback(IAudioCallback* pCallback);
extern void UnRegisterAudioCallback();

class CXAudio2 : public IAudioRenderer, public IXAudio2VoiceCallback
{
public:
	CXAudio2(IAudioCallback* pCallback, int iChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, const char* strAudioCodec = "", bool bIsMusic = false);
	CXAudio2();

	virtual void UnRegisterAudioCallback();
	virtual void RegisterAudioCallback(IAudioCallback* pCallback);
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

	bool m_bInitialized;
	bool m_bPaused;

	unsigned int m_uiSampleRate;
	long m_nCurrentVolume;
	float m_timePerPacket;
	int m_packetsSent;
	long m_lastUpdate;

	void Update();
};

#endif //H_CXAUDIO2
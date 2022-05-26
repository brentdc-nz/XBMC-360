#ifndef H_CNULLSOUND
#define H_CNULLSOUND

#include "IAudioRenderer.h"
#include "IAudioCallback.h"

extern void RegisterAudioCallback(IAudioCallback* pCallback);
extern void UnRegisterAudioCallback();

class CNullSound : public IAudioRenderer
{
public:
	CNullSound(IAudioCallback* pCallback, int iChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, const char* strAudioCodec = "", bool bIsMusic = false);
	CNullSound();

	virtual void UnRegisterAudioCallback();
	virtual void RegisterAudioCallback(IAudioCallback* pCallback);
	virtual DWORD GetChunkLen();
	virtual float GetDelay();
	virtual float GetCacheTime();

	virtual ~CNullSound();

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

private:
	long m_nCurrentVolume;

	float m_timePerPacket;
	int m_packetsSent;
	bool m_paused;
	long m_lastUpdate;

	void Update();
};

#endif //H_CNULLSOUND
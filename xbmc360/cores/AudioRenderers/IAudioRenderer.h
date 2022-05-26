#ifndef H_CAUDIORENDER
#define H_CAUDIORENDER

#ifdef _XBOX
#include <xtl.h>
#endif

#include "IAudioCallback.h"

extern void RegisterAudioCallback(IAudioCallback* pCallback);
extern void UnRegisterAudioCallback();

class IAudioRenderer
{
public:
	IAudioRenderer() {};
	virtual ~IAudioRenderer() {};
	virtual void UnRegisterAudioCallback() = 0;
	virtual void RegisterAudioCallback(IAudioCallback* pCallback) = 0;
	virtual FLOAT GetDelay() = 0;
	virtual FLOAT GetCacheTime() = 0;

	virtual DWORD AddPackets(unsigned char* data, DWORD len) = 0;
	virtual bool IsResampling() { return false;};
	virtual DWORD GetSpace() = 0;
	virtual HRESULT Deinitialize() = 0;
	virtual HRESULT Pause() = 0;
	virtual HRESULT Stop() = 0;
	virtual HRESULT Resume() = 0;
	virtual DWORD GetChunkLen() = 0;

	virtual LONG GetCurrentVolume() const = 0;
	virtual void Mute(bool bMute) = 0;
	virtual HRESULT SetCurrentVolume(LONG nVolume) = 0;
	virtual void SetDynamicRangeCompression(long drc) {};
	virtual int SetPlaySpeed(int iSpeed) = 0;
	virtual void WaitCompletion() = 0;
	virtual void DoWork() {}
	virtual void SwitchChannels(int iAudioStream, bool bAudioOnAllSpeakers) = 0;
};

#endif // H_CAUDIORENDER

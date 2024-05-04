#ifndef H_CDVDAUDIO
#define H_CDVDAUDIO

#include "..\..\utils\SingleLock.h"
#include "..\AudioRenderers\IAudioRenderer.h"
#include"..\AudioRenderers\IAudioCallback.h"

enum CodecID;
typedef struct stDVDAudioFrame DVDAudioFrame;

class CDVDAudio
{
public:
	CDVDAudio(volatile bool& bStop);
	~CDVDAudio();

	bool Create(const DVDAudioFrame &audioframe, CodecID codec);
	bool IsValidFormat(const DVDAudioFrame &audioframe);
	double GetDelay(); // Returns the time it takes to play a packet if we add one at this time
	double GetCacheTime(); // Returns total amount of data cached in audio output at this time
	DWORD AddPackets(const DVDAudioFrame &audioframe);
	void SetVolume(int iVolume);
	void Destroy();
	void Flush();
	void Finish();
	void Drain();
	void Pause();
	void Resume();

	IAudioRenderer* m_pAudioDecoder;

protected:
	DWORD AddPacketsRenderer(unsigned char* data, DWORD len, CSingleLock &lock);
	IAudioCallback* m_pCallback;
	BYTE* m_pBuffer; // Should be [m_dwPacketSize]
	DWORD m_iBufferSize;
	DWORD m_dwPacketSize;
	CCriticalSection m_critSection;

	int m_iChannels;
	int m_iBitrate;
	int m_iBitsPerSample;
	bool m_bPassthrough;
	bool m_bPaused;

	volatile bool& m_bStop;
};

#endif //H_CDVDAUDIO
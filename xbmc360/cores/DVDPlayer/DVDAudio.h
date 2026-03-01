#ifndef H_CDVDAUDIO
#define H_CDVDAUDIO

#include "utils\SingleLock.h"
#include "..\AudioRenderers\IAudioRenderer.h"
#include"..\AudioRenderers\IAudioCallback.h"
#include <queue>

enum AVCodecID;
typedef struct stDVDAudioFrame DVDAudioFrame;

class CPTSOutputQueue
{
private:
	typedef struct {double pts; double timestamp; double duration;} TPTSItem;
	TPTSItem m_current;
	std::queue<TPTSItem> m_queue;
	CCriticalSection m_sync;

public:
	CPTSOutputQueue();
	void Add(double pts, double delay, double duration, double timestamp);
	void Flush();
	double Current(double timestamp);
};

class CSingleLock;

class CDVDAudio
{
public:
	CDVDAudio(volatile bool& bStop);
	~CDVDAudio();

	void RegisterAudioCallback(IAudioCallback* pCallback);
	void UnRegisterAudioCallback();
	void DoAudioWork();


	void SetVolume(int iVolume);
	void Pause();
	void Resume();
	bool Create(const DVDAudioFrame &audioframe, AVCodecID codec);
	bool IsValidFormat(const DVDAudioFrame &audioframe);
	void Destroy();
	DWORD AddPackets(const DVDAudioFrame &audioframe);
	double GetDelay(); // Returns the time it takes to play a packet if we add one at this time
	double GetPlayingPts();
	void   SetPlayingPts(double pts);
	double GetCacheTime(); // Returns total amount of data cached in audio output at this time
	double GetCacheTotal(); // Returns total amount the audio device can buffer
	void Flush();
	void Finish();
	void Drain();

	IAudioRenderer* m_pAudioDecoder;

protected:
	CPTSOutputQueue m_time;
	DWORD AddPacketsRenderer(unsigned char* data, DWORD len, CSingleLock &lock);
	IAudioCallback* m_pCallback;
	BYTE* m_pBuffer; // Should be [m_dwPacketSize]
	DWORD m_iBufferSize;
	DWORD m_dwPacketSize;
	CCriticalSection m_critSection;

	BYTE m_visBuffer[4096];
	unsigned int m_visBufferLength;

	int m_iChannels;
	int m_iBitrate;
	int m_iBitsPerSample;
	double m_SecondsPerByte;
	bool m_bPassthrough;
	bool m_bPaused;

	volatile bool& m_bStop;
};

#endif //H_CDVDAUDIO
#ifndef H_CDVDAUDIODEVICE
#define H_CDVDAUDIODEVICE

#include <xtl.h>
#include <xaudio2.h>
#include <queue>

class CDVDAudio 
{
public:
	CDVDAudio();
	~CDVDAudio();

	bool Create(int iChannels, int iBitrate, int iBitsPerSample, bool bPasstrough);
	void Destroy();
	DWORD AddPackets(unsigned char* data, DWORD len);
	void Flush();
	float GetDelay();

private:
	bool m_bInitialized;

	std::queue<BYTE*> m_quBuffers;

    IXAudio2* m_pXAudio2;
	IXAudio2MasteringVoice* m_pMasteringVoice;

	IXAudio2SourceVoice* m_pSourceVoice;
	XAUDIO2_BUFFER m_SoundBuffer;
};

#endif //H_CDVDAUDIODEVICE
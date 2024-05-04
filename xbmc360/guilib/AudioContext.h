#ifndef GUILIB_AUDIOCONTEXT_H
#define GUILIB_AUDIOCONTEXT_H

#include <xaudio2.h>

class CAudioContext
{
public:
	CAudioContext();
	virtual ~CAudioContext();

	bool Initialize();
	IXAudio2* GetXAudio2Device();
	void SetVolume(float fLevel);
	float GetVolume();
	void DeInitialize();
	float MilliBelsToVolume(int iMilliBel); // Convert milliBels (-6000 to 0) to XAudio2 volume

protected:
	bool m_bInitialized;
	IXAudio2* m_pXAudio2;
	IXAudio2MasteringVoice* m_pMasteringVoice;	
};

extern CAudioContext g_audioContext;

#endif //GUILIB_AUDIOCONTEXT_H
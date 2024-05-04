#ifndef H_CGUISOUND
#define H_CGUISOUND

#include "AudioContext.h"
#include "..\xbox\XBAudioUtils.h"

class CGUISound
{
public:
	CGUISound();
	~CGUISound();

	bool Load(const CStdString& strFile, int iVolume);
	void Play();
	void Stop();
	bool IsPlaying();
	void SetVolume(int level);

	CStdString GetFileName() { return m_strFileName; };

private:
	bool LoadWavFile(const CStdString& strFileName);
	void FreeBuffer();

	float m_fVolume;

	IXAudio2SourceVoice* m_pSourceVoice;
	CStdString m_strFileName;

	DWORD m_dwWaveSize;
	BYTE* m_pbWaveData;
};

#endif //H_CGUISOUND
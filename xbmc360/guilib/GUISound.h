#ifndef H_CGUISOUND
#define H_CGUISOUND

#include "AudioContext.h"
#include "..\xbox\XBAudioUtils.h"

class CGUISound
{
public:
	CGUISound();
	~CGUISound();

	bool Load(const CStdString& strFile);
	void Play();
	void Stop();
	void FreeBuffer();

	CStdString GetFileName() { return m_strFileName; };

private:
	bool LoadWavFile(const CStdString& strFileName);

	IXAudio2SourceVoice* m_pSourceVoice;
	CStdString m_strFileName;

	DWORD m_dwWaveSize;
	BYTE* m_pbWaveData;
};

#endif //H_CGUISOUND
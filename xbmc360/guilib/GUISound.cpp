#include "..\utils\StdString.h"
#include "GUISound.h"
#include "GraphicContext.h"

CGUISound::CGUISound()
{
	m_pSourceVoice = NULL;
	m_strFileName = "";

	m_fVolume = 1.0;
	m_dwWaveSize = 0;
	m_pbWaveData = NULL;
}

CGUISound::~CGUISound()
{
	if(m_pSourceVoice)
		m_pSourceVoice->DestroyVoice();
}

bool CGUISound::Load(const CStdString& strFile, int iVolume)
{
	m_strFileName = strFile;
	m_fVolume = g_audioContext.MilliBelsToVolume(iVolume);

	bool bReady = LoadWavFile(strFile);

	if(!bReady)
		FreeBuffer();

	return bReady;
}

void CGUISound::Play()
{
	Stop();

	// Fill our temp buffer
	XAUDIO2_BUFFER buffer = {0};
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.pAudioData = m_pbWaveData;
	buffer.AudioBytes = m_dwWaveSize;

	m_pSourceVoice->SubmitSourceBuffer(&buffer);
	m_pSourceVoice->SetVolume(m_fVolume);
	m_pSourceVoice->Start(0);
}

void CGUISound::Stop()
{
	m_pSourceVoice->Stop();
}

bool CGUISound::IsPlaying()
{
	if (m_pSourceVoice)
	{
		XAUDIO2_VOICE_STATE state;
		m_pSourceVoice->GetState(&state);

		if(state.BuffersQueued > 0)
			return true;
	}

	return false;
}

void CGUISound::SetVolume(int level)
{
	m_fVolume = g_audioContext.MilliBelsToVolume(level);
}

bool CGUISound::LoadWavFile(const CStdString& strFileName)
{
	HRESULT hr;
	IXAudio2* pXAudio2 = NULL;
	pXAudio2 = g_audioContext.GetXAudio2Device();

	if(!pXAudio2)
		return false;

	CStdString strSoundsPath = g_graphicsContext.GetMediaDir() + "\\sounds\\" + strFileName;

	ATG::WaveFile WaveFile;

	// Read the wave file
	if(FAILED(hr = WaveFile.Open(strSoundsPath.c_str())))
		return false;

	WAVEFORMATEXTENSIBLE wfx = {0};

	// Read the format header
	WaveFile.GetFormat(&wfx);

	// Calculate how many bytes and samples are in the wave
	WaveFile.GetDuration(&m_dwWaveSize);

	// Read the sample data into memory
	m_pbWaveData = new BYTE[m_dwWaveSize];
	WaveFile.ReadSample(0, m_pbWaveData, m_dwWaveSize, &m_dwWaveSize);

	if(FAILED(hr = pXAudio2->CreateSourceVoice(&m_pSourceVoice, (WAVEFORMATEX*)&wfx)))
		return false;

	return true;
}

void CGUISound::FreeBuffer()
{
	if(m_pbWaveData)
		SAFE_DELETE_ARRAY(m_pbWaveData);
}
#include "AudioContext.h"
#include "..\utils\Log.h"

CAudioContext g_audioContext;

CAudioContext::CAudioContext()
{
	m_bInitialized = false;
	m_pXAudio2 = NULL;
	m_pMasteringVoice = NULL;	
}

CAudioContext::~CAudioContext()
{
}

bool CAudioContext::Initialize()
{
	if(m_bInitialized)
		return true;

	UINT32 flags = 0;

	if(XAudio2Create(&m_pXAudio2, flags ) != S_OK)
	{
		CLog::Log(LOGERROR, "Failed to create XAudio2 object.");
		return false;
	}

	if(m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice ) != S_OK)
 	{
		CLog::Log(LOGERROR, "Failed to create XAudio2 mastering voice.");
		return false;
	}

	m_bInitialized = true;

	return true;
}

IXAudio2* CAudioContext::GetXAudio2Device()
{
	return m_pXAudio2;
}

void CAudioContext::SetVolume(float fLevel)
{
	if(!m_bInitialized)
		return;
	
	m_pMasteringVoice->SetVolume(fLevel);
}

float CAudioContext::GetVolume()
{
	float fVolume = 0;

	m_pMasteringVoice->GetVolume(&fVolume);

	return fVolume;
}

void CAudioContext::DeInitialize()
{
	if(m_pMasteringVoice)
		m_pMasteringVoice->DestroyVoice();

	if(m_pXAudio2)
		m_pXAudio2->Release();

	m_bInitialized = false;
}

float CAudioContext::MilliBelsToVolume(int iMilliBel) // Convert milliBels (-6000 to 0) to XAudio2 volume
{
    // Convert milliBel to the corresponding float value in the range [0.0, 1.0] using a logarithmic scale
    float fResult = pow(10.0f, static_cast<float>(iMilliBel) / 2000.0f); // 2000 is approximately log10(1000)

    // Normalize the result to the [0.0, 1.0] range
    fResult = (fResult - pow(10.0f, static_cast<float>(-6000) / 2000.0f)) /
             (pow(10.0f, static_cast<float>(0) / 2000.0f) -
              pow(10.0f, static_cast<float>(-6000) / 2000.0f));

    return fResult;
}
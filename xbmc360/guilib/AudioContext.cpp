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

void CAudioContext::DeInitialize()
{
	if(m_pMasteringVoice)
		m_pMasteringVoice->DestroyVoice();

	if(m_pXAudio2)
		m_pXAudio2->Release();

	m_bInitialized = false;
}
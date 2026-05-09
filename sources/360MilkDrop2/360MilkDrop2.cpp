#include <xtl.h>
#include "xbox\legacyshaders.h"
#include "vis_milkdrop2\plugin.h"
#include "360MilkDrop2.h"

using namespace std;

bool g_bInitialized = false;
CPlugin* g_pPlugin = NULL;
CRITICAL_SECTION g_critSection;
bool g_bCritSectionInitialized = false;

unsigned char waves[2][576];

CVisualisationMilkDrop2::CVisualisationMilkDrop2(const std::string strVisualisationName) : CVisualisation(strVisualisationName)
{
	m_presetNames = NULL;
	m_numPresets = 0;
}

CVisualisationMilkDrop2::~CVisualisationMilkDrop2()
{
	FreePresetNames();
}

void CVisualisationMilkDrop2::Create(LPDIRECT3DDEVICE9 pd3dDevice, int iPosX, int iPosY, int iWidth, int iHeight)
{
	if (!g_bCritSectionInitialized)
	{
		InitializeCriticalSection(&g_critSection);
		g_bCritSectionInitialized = true;
	}

	EnterCriticalSection(&g_critSection);
	if(!g_bInitialized)
	{
		CLegacyShaders::CreateShaders(pd3dDevice);

		// Create Milkdrop 2
		g_pPlugin = new CPlugin;
		g_pPlugin->PluginPreInitialize();
		g_pPlugin->PluginInitialize(pd3dDevice, iPosX, iPosY, iWidth, iHeight);
	
		g_bInitialized = true;
	}
	LeaveCriticalSection(&g_critSection);
}

void CVisualisationMilkDrop2::AudioData(const short* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength)
{
	int ipos=0;
	
	while (ipos < 576)
	{
		for (int i=0; i < iAudioDataLength; i+=2)
		{
			// pAudioData is signed 16-bit PCM (-32768..+32767), interleaved L/R.
			// MilkDrop's AnalyzeNewSound expects unsigned 8-bit (0..255) where 128 = silence.
			// Convert: shift signed range to unsigned (0..65535), then take the high byte.
			waves[0][ipos] = (unsigned char)((pAudioData[i]   + 32768) >> 8);
			waves[1][ipos] = (unsigned char)((pAudioData[i+1] + 32768) >> 8);
			ipos++;
			if (ipos >= 576) break;
		}
	}
}

void CVisualisationMilkDrop2::Render()
{
	if(g_bInitialized && g_bCritSectionInitialized)
	{
		EnterCriticalSection(&g_critSection);
		if(g_bInitialized && g_pPlugin)
		{
			g_pPlugin->PluginRender(waves[0], waves[1]);
		}
		LeaveCriticalSection(&g_critSection);
	}
}

void CVisualisationMilkDrop2::Stop() // FIXME ME : We have other shutdown functions to run in CPlugin
{
	if (g_bCritSectionInitialized)
		EnterCriticalSection(&g_critSection);

	if (g_bInitialized)
	{
		// Unbind shaders from the device before releasing them
		if (g_pPlugin)
		{
			D3DDevice* pDevice = g_pPlugin->GetDevice();
			if (pDevice)
			{
				CLegacyShaders::UnbindShaders(pDevice);
			}
		}

		// Clean up plugin's D3D resources
		if (g_pPlugin)
			g_pPlugin->CleanUpMyDX9Stuff(1);

		// Now safe to delete shaders
		CLegacyShaders::DeleteShaders();

		// Delete the plugin
		if(g_pPlugin)
			delete g_pPlugin;

		g_pPlugin = NULL;
		
		g_bInitialized = false;
	}

	if (g_bCritSectionInitialized)
	{
		LeaveCriticalSection(&g_critSection);
		DeleteCriticalSection(&g_critSection);
		g_bCritSectionInitialized = false;
	}
}

void CVisualisationMilkDrop2::BuildPresetNames()
{
	FreePresetNames();

	if (!g_pPlugin || !g_bInitialized)
		return;

	int nDirs = g_pPlugin->m_nDirs;
	int nTotal = g_pPlugin->m_nPresets;
	m_numPresets = nTotal - nDirs;

	if (m_numPresets <= 0)
	{
		m_numPresets = 0;
		return;
	}

	m_presetNames = new char*[m_numPresets];
	for (int i = 0; i < m_numPresets; i++)
	{
		const std::string &name = g_pPlugin->m_presets[nDirs + i].szFilename;
		m_presetNames[i] = new char[name.size() + 1];
		strcpy(m_presetNames[i], name.c_str());
	}
}

void CVisualisationMilkDrop2::FreePresetNames()
{
	if (m_presetNames)
	{
		for (int i = 0; i < m_numPresets; i++)
			delete[] m_presetNames[i];
		delete[] m_presetNames;
		m_presetNames = NULL;
	}
	m_numPresets = 0;
}

bool CVisualisationMilkDrop2::OnAction(VIS_ACTION action, void *param)
{
	if (!g_pPlugin || !g_bInitialized)
		return false;

	switch (action)
	{
	case VIS_ACTION_NEXT_PRESET:
		g_pPlugin->NextPreset(g_pPlugin->m_fBlendTimeUser);
		return true;
	case VIS_ACTION_PREV_PRESET:
		g_pPlugin->PrevPreset(g_pPlugin->m_fBlendTimeUser);
		return true;
	case VIS_ACTION_LOCK_PRESET:
		g_pPlugin->m_bPresetLockedByUser = !g_pPlugin->m_bPresetLockedByUser;
		return true;
	case VIS_ACTION_RANDOM_PRESET:
		g_pPlugin->LoadRandomPreset(g_pPlugin->m_fBlendTimeUser);
		return true;
	case VIS_ACTION_LOAD_PRESET:
		if (param)
		{
			int index = *(int *)param;
			int nDirs = g_pPlugin->m_nDirs;
			if (index >= 0 && index < (g_pPlugin->m_nPresets - nDirs))
			{
				char szFile[MAX_PATH] = {0};
				strcpy(szFile, g_pPlugin->m_szPresetDir);
				strcat(szFile, g_pPlugin->m_presets[nDirs + index].szFilename.c_str());
				g_pPlugin->LoadPreset(szFile, g_pPlugin->m_fBlendTimeUser);
			}
		}
		return true;
	default:
		break;
	}
	return false;
}

void CVisualisationMilkDrop2::GetPresets(char ***pPresets, int *currentPreset, int *numPresets, bool *locked)
{
	if (!g_pPlugin || !g_bInitialized)
	{
		if (pPresets) *pPresets = NULL;
		if (currentPreset) *currentPreset = 0;
		if (numPresets) *numPresets = 0;
		if (locked) *locked = false;
		return;
	}

	// Rebuild preset name cache if count changed
	int nDirs = g_pPlugin->m_nDirs;
	int count = g_pPlugin->m_nPresets - nDirs;
	if (count != m_numPresets)
		BuildPresetNames();

	if (pPresets) *pPresets = m_presetNames;
	if (numPresets) *numPresets = m_numPresets;
	if (locked) *locked = g_pPlugin->m_bPresetLockedByUser;
	if (currentPreset)
	{
		int cur = g_pPlugin->m_nCurrentPreset - nDirs;
		*currentPreset = (cur >= 0 && cur < m_numPresets) ? cur : 0;
	}
}

void CVisualisationMilkDrop2::GetCurrentPreset(char **pPreset, bool *locked)
{
	if (pPreset && locked)
	{
		char **presets = NULL;
		int currentPreset = 0;
		int numPresets = 0;
		*locked = false;
		GetPresets(&presets, &currentPreset, &numPresets, locked);
		if (presets && currentPreset < numPresets)
			*pPreset = presets[currentPreset];
		else
			*pPreset = NULL;
	}
}

bool CVisualisationMilkDrop2::IsLocked()
{
	char *preset = NULL;
	bool locked = false;
	GetCurrentPreset(&preset, &locked);
	return locked;
}

char *CVisualisationMilkDrop2::GetPreset()
{
	char *preset = NULL;
	bool locked = false;
	GetCurrentPreset(&preset, &locked);
	return preset;
}
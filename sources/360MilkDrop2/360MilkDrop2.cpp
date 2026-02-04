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
}

CVisualisationMilkDrop2::~CVisualisationMilkDrop2()
{
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
		g_pPlugin->PluginInitialize(pd3dDevice, iPosX, iPosX, iWidth, iHeight);
	
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
			waves[0][ipos] = char ((pAudioData[i] / 65535.0f) * 255.0f);
			waves[1][ipos] = char ((pAudioData[i+1] / 65535.0f) * 255.0f);
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
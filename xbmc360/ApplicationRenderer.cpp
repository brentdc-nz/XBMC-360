//#include "system.h"
#include "utils\log.h"
#include "Application.h"
#include "ApplicationRenderer.h"
#include "guilib\GUIImage.h"
//#include "settings\AdvancedSettings.h"
#include "guilib\GUIWindowManager.h"
#include "utils\SingleLock.h"

CApplicationRenderer g_ApplicationRenderer;

CApplicationRenderer::CApplicationRenderer(void)
{
}

CApplicationRenderer::~CApplicationRenderer()
{
	Stop();
}

void CApplicationRenderer::OnStartup()
{
	m_time = CTimeUtils::timeGetTime();
	m_enabled = true;
	m_busyShown = false;
	m_explicitbusy = 0;
	m_busycount = 0;
	m_prevbusycount = 0;
	m_lpSurface = NULL;
	m_pWindow = NULL;
	m_Resolution = g_graphicsContext.GetVideoResolution();
}

void CApplicationRenderer::OnExit()
{
	m_busycount = m_prevbusycount = m_explicitbusy = 0;
	m_busyShown = false;
	
	if (m_pWindow) m_pWindow->Close(true);
		m_pWindow = NULL;
	
	SAFE_RELEASE(m_lpSurface);
}

void CApplicationRenderer::Process()
{
	int iWidth = 0;
	int iHeight = 0;
	int iLeft = 0;
	int iTop = 0;
	LPDIRECT3DSURFACE9 lpSurfaceBack = NULL;
	LPDIRECT3DSURFACE9 lpSurfaceFront = NULL;

	while (!m_bStop)
	{
		if (!m_enabled || g_graphicsContext.IsFullScreenVideo())
		{
			Sleep(50);
			continue;
		}

		// TODO

		Sleep(1);
	}
}

bool CApplicationRenderer::CopySurface(LPDIRECT3DSURFACE9 pSurfaceSource, const RECT* rcSource, LPDIRECT3DSURFACE9 pSurfaceDest, const RECT* rcDest)
{
	// TODO
	return false;
}

void CApplicationRenderer::UpdateBusyCount()
{
	if (m_busycount == 0)
	{
		m_prevbusycount = 0;
	}
	else
	{
		m_busycount--;
		m_prevbusycount = m_busycount;

		if (m_pWindow && m_busyShown)
		{
			m_busyShown = false;
			m_pWindow->Close();
		}
	}
}

void CApplicationRenderer::Render(bool bFullscreen)
{
	CSingleLock lockg (g_graphicsContext);
	Disable();
	UpdateBusyCount();
	SAFE_RELEASE(m_lpSurface);
	
	if(bFullscreen)
	{
		g_application.DoRenderFullScreen();
	}
	else
	{
		g_application.DoRender();
	}

	m_time = CTimeUtils::timeGetTime();
	Enable();
}

void CApplicationRenderer::Enable()
{
	m_enabled = true;
}

void CApplicationRenderer::Disable()
{
	m_enabled = false;
}

bool CApplicationRenderer::Start()
{
	if (/*g_advancedSettings.m_busyDialogDelay <= 0*/0) return false; // Delay of 0 is considered disabled // TODO
	Create();
	return true;
}

void CApplicationRenderer::Stop()
{
	StopThread();
}

bool CApplicationRenderer::IsBusy() const
{
	return ((m_explicitbusy > 0) || m_busyShown);
}

void CApplicationRenderer::SetBusy(bool bBusy) // TODO
{
	if (/*g_advancedSettings.m_busyDialogDelay <= 0*/0) return; // Never show it if disabled // TODO
	bBusy?m_explicitbusy++:m_explicitbusy--;
	
	if (m_explicitbusy < 0) m_explicitbusy = 0;

	if (m_pWindow) 
	{
		if (m_explicitbusy > 0)
			m_pWindow->Show();
		else 
			m_pWindow->Close();
	}
}

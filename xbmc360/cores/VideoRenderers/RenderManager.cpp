#include "RenderManager.h"
#include "Application.h"
#include "utils\Log.h"
#include "guilib\GraphicContext.h"

CRenderManager g_renderManager;

#define MAXPRESENTDELAY 500

CRenderManager::CRenderManager()
{
	m_pRenderer = NULL;
	m_bPauseDrawing = false;
	m_bIsStarted = false;
	m_dwPresentTime = 0;
}

CRenderManager::~CRenderManager()
{
	m_pRenderer = NULL;
	m_bIsStarted = false;	  
}

void CRenderManager::Update(bool bPauseDrawing)
{
	DWORD locks = ExitCriticalSection(g_graphicsContext);
	CExclusiveLock lock(m_sharedSection);
	RestoreCriticalSection(g_graphicsContext, locks);

	m_bPauseDrawing = bPauseDrawing;
	if (m_pRenderer)
	{
		m_pRenderer->Update(bPauseDrawing);
	}
}

void CRenderManager::RenderUpdate(bool clear, DWORD flags, DWORD alpha)
{
	if (m_pRenderer)
		m_pRenderer->RenderUpdate(clear, flags, alpha);
}

bool CRenderManager::PreInit()
{
	DWORD locks = ExitCriticalSection(g_graphicsContext);
	CExclusiveLock lock(m_sharedSection);
	RestoreCriticalSection(g_graphicsContext, locks);

	m_bIsStarted = false;
	m_bPauseDrawing = false;

	if (!m_pRenderer)
	{ 
		// No renderer
		
		// ONLY HAVE THE RGB Renderer ATM
		//m_rendermethod = g_guiSettings.GetInt("videoplayer.rendermethod");

		if (/*m_rendermethod == RENDER_HQ_RGB_SHADER*/1)
		{
			CLog::Log(LOGDEBUG, __FUNCTION__" - Selected RGB-Renderer");
			m_pRenderer = (CBaseRenderer*) new CRGBRenderer(g_graphicsContext.Get3DDevice());
		}
	}

	return m_pRenderer->PreInit();
}

bool CRenderManager::Configure(int width, int height, unsigned flags)
{
	DWORD locks = ExitCriticalSection(g_graphicsContext);
	CExclusiveLock lock(m_sharedSection);      

	if(!m_pRenderer) 
	{
		RestoreCriticalSection(g_graphicsContext, locks);
		CLog::Log(LOGERROR, "%s called without a valid Renderer object", __FUNCTION__);
		return false;
	}

	bool result = m_pRenderer->Configure(width, height/*, d_width, d_height, fps, flags*/);
	if(result)
	{
		if(flags & CONF_FLAGS_FULLSCREEN)
		{
//			lock.Leave();
			g_application.getApplicationMessenger().SwitchToFullscreen();
//			lock.Enter();
		}
//		m_pRenderer->Update(false);
		m_bIsStarted = true;
	}
  
	RestoreCriticalSection(g_graphicsContext, locks);

	return result;
}

bool CRenderManager::IsConfigured()
{
	if(!m_pRenderer)
		return false;

	return m_pRenderer->IsConfigured();
}

bool CRenderManager::GetImage(YV12Image* image)
{
	if(!m_pRenderer) 
	{
		CLog::Log(LOGERROR, "%s called without a valid Renderer object", __FUNCTION__);
		return false;
	}
	return m_pRenderer->GetImage(image);
}

void CRenderManager::ReleaseImage()
{
	if(!m_pRenderer) 
	{
		CLog::Log(LOGERROR, "%s called without a valid Renderer object", __FUNCTION__);
		return;
	}
	m_pRenderer->ReleaseImage();
}

void CRenderManager::PrepareDisplay()
{
    CSharedLock lock(m_sharedSection);

//	if (m_bPauseDrawing) return;

    if (m_pRenderer)
		m_pRenderer->PrepareDisplay();
}

void CRenderManager::FlipPage(DWORD delay /* = 0LL*/, int source /*= -1*/, EFIELDSYNC sync /*= FS_NONE*/)
{
	DWORD timestamp = 0;

	if(delay > MAXPRESENTDELAY) delay = MAXPRESENTDELAY;

	if(delay > 0)
		timestamp = GetTickCount() + delay;


	if(!m_pRenderer) 
	{
		CLog::Log(LOGERROR, "%s called without a valid Renderer object", __FUNCTION__);
		return;
	}

	while(timestamp > GetTickCount()/* && !CThread::m_bStop*/)
		Sleep(1);

	m_pRenderer->FlipPage();
}

void CRenderManager::UnInit()
{
	m_bIsStarted = false;

	if (m_pRenderer)
	{
		m_pRenderer->UnInit();
		delete m_pRenderer; 
		m_pRenderer = NULL; 
	}
}


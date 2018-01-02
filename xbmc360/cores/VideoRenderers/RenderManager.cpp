#include <xtl.h>
#include "RenderManager.h"
#include "..\..\Application.h"
#include "..\..\utils\Log.h"
#include "..\..\guilib\GraphicContext.h"

CRenderManager::CRenderManager()
{
	m_pRenderer = NULL;
	m_bIsStarted = false;
}

CRenderManager::~CRenderManager()
{
	m_pRenderer = NULL;
	m_bIsStarted = false;	  
}

void CRenderManager::RenderUpdate(bool clear, DWORD flags, DWORD alpha)
{
  if (m_pRenderer)
    m_pRenderer->RenderUpdate(clear, flags, alpha);
}

bool CRenderManager::PreInit()
{
	//FIXME Lock and release!

	m_bIsStarted = false;

	if (!m_pRenderer)
	{ 
		// no renderer
		
		//ONLY HAVE THE RGB Renderer ATM
		//m_rendermethod = g_guiSettings.GetInt("videoplayer.rendermethod");

		if (/*m_rendermethod == RENDER_HQ_RGB_SHADER*/1)
		{
			CLog::Log(LOGDEBUG, __FUNCTION__" - Selected RGB-Renderer");
			m_pRenderer = (CBaseRenderer*) new CRGBRenderer(g_graphicsContext.Get3DDevice());
		}
	}

	return m_pRenderer->PreInit();
}

bool CRenderManager::Configure(int width, int height)
{
//	DWORD locks = ExitCriticalSection(g_graphicsContext);
//	CExclusiveLock lock(m_sharedSection);      

	if(!m_pRenderer) 
	{
//		RestoreCriticalSection(g_graphicsContext, locks);
		CLog::Log(LOGERROR, "%s called without a valid Renderer object", __FUNCTION__);
		return false;
	}

	bool result = m_pRenderer->Configure(width, height/*, d_width, d_height, fps, flags*/);
	if(result)
	{
		if( /*flags & CONF_FLAGS_FULLSCREEN*/1 ) //Marty - Always true atm
		{
//			lock.Leave();
			g_application./*getApplicationMessenger().*/SwitchToFullScreen();
//			lock.Enter();
		}
//		m_pRenderer->Update(false);
		m_bIsStarted = true;
	}
  
//	RestoreCriticalSection(g_graphicsContext, locks);

	return result;
}

bool CRenderManager::GetImage(RGB32Image_t* image)
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
//    if (m_bPauseDrawing) return;
    if (m_pRenderer)
    {
      m_pRenderer->PrepareDisplay();
    }
}

void CRenderManager::FlipPage()
{
	if(!m_pRenderer) 
	{
		CLog::Log(LOGERROR, "%s called without a valid Renderer object", __FUNCTION__);
		return;
	}
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

CRenderManager g_renderManager;
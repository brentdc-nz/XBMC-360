#include "GraphicContext.h"
#include "utils\Log.h"
#include "Settings.h"
#include "GuiSettings.h"
#include "SkinInfo.h"
#include "GUIFontManager.h"
#include "xbox\XBVideoConfig.h"

CGraphicContext g_graphicsContext;

// Quick access to a skin setting, fine unless we starts clearing video settings
static CSettingInt* g_guiSkinzoom = NULL;

CGraphicContext::CGraphicContext(void)
{
	m_iScreenWidth = 852;
	m_iScreenHeight = 480;
	m_pd3dDevice = NULL;
	m_pd3dParams = NULL;
	m_strMediaDir = "";
	m_Resolution = INVALID;
	m_guiScaleX = m_guiScaleY = 1.0f;
	m_windowResolution = INVALID;
}

CGraphicContext::~CGraphicContext(void)
{
	while (m_viewStack.size())
	{
		D3DVIEWPORT9 *viewport = m_viewStack.top();
		m_viewStack.pop();
		if (viewport) delete viewport;
	}
}

void CGraphicContext::TLock()
{
	EnterCriticalSection(*this);

	if(m_pd3dDevice)
		m_pd3dDevice->AcquireThreadOwnership();
}

void CGraphicContext::TUnlock()
{
	m_pd3dDevice->ReleaseThreadOwnership();

	LeaveCriticalSection(*this);
}

void CGraphicContext::SetD3DDevice(LPDIRECT3DDEVICE9 p3dDevice)
{
	m_pd3dDevice = p3dDevice;

	// Release the object for any thread to acquire!
	if(m_pd3dDevice)
		m_pd3dDevice->ReleaseThreadOwnership();
}

void CGraphicContext::SetD3DParameters(D3DPRESENT_PARAMETERS *p3dParams)
{
	m_pd3dParams = p3dParams;
}

void CGraphicContext::SetVideoResolution(RESOLUTION &res, BOOL NeedZ, bool forceClear /* = false */)
{
	if (res == AUTORES)
		res = g_videoConfig.GetBestMode();

	if (!g_videoConfig.IsValidResolution(res))
	{
		// Choose a failsafe resolution that we can actually display
		CLog::Log(LOGERROR, "The screen resolution requested is not valid, resetting to a valid mode");
		res = g_videoConfig.GetSafeMode();
	}

	if(!m_pd3dParams)
	{
		m_Resolution = res;
		return;
	}

	bool NeedReset = false;

	UINT interval = D3DPRESENT_INTERVAL_ONE;

#ifdef PROFILE
	interval = D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

#ifndef HAS_XBOX_D3D
	interval = 0;
#endif

	if(NeedZ != m_pd3dParams->EnableAutoDepthStencil)
	{
		m_pd3dParams->EnableAutoDepthStencil = NeedZ;
		NeedReset = true;
	}

	if(m_Resolution != res)
	{
		NeedReset = true;
		m_pd3dParams->BackBufferWidth = g_settings.m_ResInfo[res].iWidth;
		m_pd3dParams->BackBufferHeight = g_settings.m_ResInfo[res].iHeight;
		m_pd3dParams->Flags = g_settings.m_ResInfo[res].dwFlags;

		if(res == HDTV_1080p || res == HDTV_720p || m_bFullScreenVideo)
			m_pd3dParams->BackBufferCount = 1;
		else
			m_pd3dParams->BackBufferCount = 2;

		if(res == PAL60_4x3 || res == PAL60_16x9)
		{
			if(m_pd3dParams->BackBufferWidth <= 720 && m_pd3dParams->BackBufferHeight <= 480)
			{
				m_pd3dParams->FullScreen_RefreshRateInHz = 60;
			}
			else
			{
				m_pd3dParams->FullScreen_RefreshRateInHz = 0;
			}
		}
		else
		{
			m_pd3dParams->FullScreen_RefreshRateInHz = 0;
		}
	}

	Lock();

	if(m_pd3dDevice)
	{
		if(NeedReset)
		{
			CLog::Log(LOGDEBUG, "Setting resolution %i", res);
			m_pd3dDevice->Reset(m_pd3dParams);
		}

		// Need to clear and preset, otherwise flicker filters won't take effect
		if(NeedReset || forceClear)
		{
			g_graphicsContext.TLock();
			m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00010001, 1.0f, 0L );
			g_graphicsContext.TUnlock();
			g_graphicsContext.TLock();
			m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
			g_graphicsContext.TUnlock();
		}

		m_iScreenWidth = m_pd3dParams->BackBufferWidth;
		m_iScreenHeight = m_pd3dParams->BackBufferHeight;
	}

//	SetFullScreenViewWindow(res);
//	SetScreenFilters(m_bFullScreenVideo);

	m_Resolution = res;

	if(NeedReset)
	{
		CLog::Log(LOGDEBUG, "We set resolution %i", m_Resolution);
		if(m_Resolution != INVALID)
			g_fontManager.ReloadTTFFonts();
	}

	Unlock();
}

RESOLUTION CGraphicContext::GetVideoResolution() const
{
	return m_Resolution;
}

void CGraphicContext::ResetOverscan(RESOLUTION res, OVERSCAN &overscan)
{
	overscan.left = 0;
	overscan.top = 0;

	switch(res)
	{
		case HDTV_1080p:
			overscan.right = 1920;
			overscan.bottom = 1080;
		break;

		case HDTV_720p:
			overscan.right = 1280;
			overscan.bottom = 720;
		break;

		case HDTV_480p_16x9:
		case HDTV_480p_4x3:
		case NTSC_16x9:
		case NTSC_4x3:
		case PAL60_16x9:
		case PAL60_4x3:
			overscan.right = 720;
			overscan.bottom = 480;
		break;

		case PAL_16x9:
		case PAL_4x3:
			overscan.right = 720;
			overscan.bottom = 576;
		break;

		default:
		break;
	}
}

int CGraphicContext::GetFPS() const
{
	if (m_Resolution == PAL_4x3 || m_Resolution == PAL_16x9)
		return 50;
	else if (m_Resolution == HDTV_1080p)
		return 30;

	return 60;
}

const RECT& CGraphicContext::GetViewWindow() const
{
	return m_videoRect;
}

void CGraphicContext::SetViewWindow(float left, float top, float right, float bottom)
{
    m_videoRect.left = (long)(ScaleFinalXCoord(left, top) + 0.5f);
    m_videoRect.top = (long)(ScaleFinalYCoord(left, top) + 0.5f);
    m_videoRect.right = (long)(ScaleFinalXCoord(right, bottom) + 0.5f);
    m_videoRect.bottom = (long)(ScaleFinalYCoord(right, bottom) + 0.5f);

    if (m_bShowPreviewWindow && !m_bFullScreenVideo)
    {
		D3DRECT d3dRC;
		d3dRC.x1 = m_videoRect.left;
		d3dRC.x2 = m_videoRect.right;
		d3dRC.y1 = m_videoRect.top;
		d3dRC.y2 = m_videoRect.bottom;

		TLock();
		Get3DDevice()->Clear(1, &d3dRC, D3DCLEAR_TARGET, 0x00010001, 1.0f, 0L);
		TUnlock();
    }
}

bool CGraphicContext::SetViewPort(float fx, float fy , float fwidth, float fheight, bool intersectPrevious /* = false */)
{
	D3DVIEWPORT9 newviewport;
	D3DVIEWPORT9 *oldviewport = new D3DVIEWPORT9;

	TLock();
	Get3DDevice()->GetViewport(oldviewport);
	TUnlock();
	
	// Transform coordinates - we may have a rotation which changes the positioning of the
	// minimal and maximal viewport extents. We currently go to the maximal extent.
	float x[4], y[4];
	x[0] = x[3] = fx;
	x[1] = x[2] = fx + fwidth;
	y[0] = y[1] = fy;
	y[2] = y[3] = fy + fheight;
	float minX = (float)m_iScreenWidth;
	float maxX = 0;
	float minY = (float)m_iScreenHeight;
	float maxY = 0;
	
	for (int i = 0; i < 4; i++)
	{
		float z = 0;
		ScaleFinalCoords(x[i], y[i], z);
		if (x[i] < minX) minX = x[i];
		if (x[i] > maxX) maxX = x[i];
		if (y[i] < minY) minY = y[i];
		if (y[i] > maxY) maxY = y[i];
  }

	int newLeft = (int)(minX + 0.5f);
	int newTop = (int)(minY + 0.5f);
	int newRight = (int)(maxX + 0.5f);
	int newBottom = (int)(maxY + 0.5f);
	
	if (intersectPrevious)
	{
		// Do the intersection
		int oldLeft = (int)oldviewport->X;
		int oldTop = (int)oldviewport->Y;
		int oldRight = (int)oldviewport->X + oldviewport->Width;
		int oldBottom = (int)oldviewport->Y + oldviewport->Height;
	
		if (newLeft >= oldRight || newTop >= oldBottom || newRight <= oldLeft || newBottom <= oldTop)
		{
			// Empty intersection - return false to indicate no rendering should occur
#if defined(HAS_SDL_OPENGL)
			delete [] oldviewport;
#else
			delete oldviewport;
#endif
			return false;
		}
		
		// ok, they intersect, do the intersection
		if (newLeft < oldLeft) newLeft = oldLeft;
		if (newTop < oldTop) newTop = oldTop;
		if (newRight > oldRight) newRight = oldRight;
		if (newBottom > oldBottom) newBottom = oldBottom;
	}
	
	// Check range against screen size
	if (newRight <= 0 || newBottom <= 0 ||
		newTop >= m_iScreenHeight || newLeft >= m_iScreenWidth ||
		newLeft >= newRight || newTop >= newBottom)
	{
		// no intersection with the screen
#if defined(HAS_SDL_OPENGL)
		delete [] oldviewport;
#else
		delete oldviewport;
#endif
		return false;
	}
	
	// Intersection with the screen
	if (newLeft < 0) newLeft = 0;
	if (newTop < 0) newTop = 0;
	if (newRight > m_iScreenWidth) newRight = m_iScreenWidth;
	if (newBottom > m_iScreenHeight) newBottom = m_iScreenHeight;

	ASSERT(newLeft < newRight);
	ASSERT(newTop < newBottom);

	newviewport.MinZ = 0.0f;
	newviewport.MaxZ = 1.0f;
	newviewport.X = newLeft;
	newviewport.Y = newTop;
	newviewport.Width = newRight - newLeft;
	newviewport.Height = newBottom - newTop;

	TLock();
	m_pd3dDevice->SetViewport(&newviewport);
	TUnlock();
	m_viewStack.push(oldviewport);

	UpdateCameraPosition(m_cameras.top());
	return true;
}

void CGraphicContext::RestoreViewPort()
{
	if (!m_viewStack.size()) return;
	
	D3DVIEWPORT9 *oldviewport = (D3DVIEWPORT9*)m_viewStack.top();
	m_viewStack.pop();

	TLock();
	Get3DDevice()->SetViewport(oldviewport);
	TUnlock();

	if (oldviewport)
	{
#if defined(HAS_SDL_OPENGL)
		delete [] oldviewport;
#else
		delete oldviewport;
#endif
	}

	UpdateCameraPosition(m_cameras.top());
}

void CGraphicContext::SetFullScreenVideo(bool bOnOff)
{
	Lock();
	m_bFullScreenVideo = bOnOff;
//	SetFullScreenViewWindow(m_Resolution);
	Unlock();
}

bool CGraphicContext::IsFullScreenVideo() const
{
	return m_bFullScreenVideo;
}

void CGraphicContext::SetMediaDir(const CStdString &strMediaDir)
{
	g_TextureManager.SetTexturePath(strMediaDir);
	m_strMediaDir = strMediaDir;
}

void CGraphicContext::Lock()
{
	if(!m_pd3dDevice)
		return;

	EnterCriticalSection(*this);

//	m_pd3dDevice->AcquireThreadOwnership();
}


void CGraphicContext::Unlock()
{
	if(!m_pd3dDevice)
		return;
	
//	m_pd3dDevice->ReleaseThreadOwnership();

	LeaveCriticalSection(*this);
}

void CGraphicContext::ResetScreenParameters(RESOLUTION res)
{
  // 1080i
  switch (res)
  {
  case HDTV_1080p:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.965 * 1080);
    g_settings.m_ResInfo[res].iWidth = 1920;
    g_settings.m_ResInfo[res].iHeight = 1080;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_INTERLACED | D3DPRESENTFLAG_WIDESCREEN;
    g_settings.m_ResInfo[res].fPixelRatio = 1.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "1080i 16:9");
    break;
  case HDTV_720p:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.965 * 720);
    g_settings.m_ResInfo[res].iWidth = 1280;
    g_settings.m_ResInfo[res].iHeight = 720;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_PROGRESSIVE | D3DPRESENTFLAG_WIDESCREEN;
    g_settings.m_ResInfo[res].fPixelRatio = 1.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "720p 16:9");
    break;
  case HDTV_480p_4x3:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.9 * 480);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 480;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_PROGRESSIVE;
    g_settings.m_ResInfo[res].fPixelRatio = 4320.0f / 4739.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "480p 4:3");
    break;
  case HDTV_480p_16x9:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.965 * 480);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 480;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_PROGRESSIVE | D3DPRESENTFLAG_WIDESCREEN;
    g_settings.m_ResInfo[res].fPixelRatio = 4320.0f / 4739.0f*4.0f / 3.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "480p 16:9");
    break;
  case NTSC_4x3:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.9 * 480);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 480;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_INTERLACED;
    g_settings.m_ResInfo[res].fPixelRatio = 4320.0f / 4739.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "NTSC 4:3");
    break;
  case NTSC_16x9:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.965 * 480);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 480;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_INTERLACED | D3DPRESENTFLAG_WIDESCREEN;
    g_settings.m_ResInfo[res].fPixelRatio = 4320.0f / 4739.0f*4.0f / 3.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "NTSC 16:9");
    break;
  case PAL_4x3:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.9 * 576);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 576;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_INTERLACED;
    g_settings.m_ResInfo[res].fPixelRatio = 128.0f / 117.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "PAL 4:3");
    break;
  case PAL_16x9:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.965 * 576);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 576;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_INTERLACED | D3DPRESENTFLAG_WIDESCREEN;
    g_settings.m_ResInfo[res].fPixelRatio = 128.0f / 117.0f*4.0f / 3.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "PAL 16:9");
    break;
  case PAL60_4x3:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.9 * 480);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 480;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_INTERLACED;
    g_settings.m_ResInfo[res].fPixelRatio = 4320.0f / 4739.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "PAL60 4:3");
    break;
  case PAL60_16x9:
    g_settings.m_ResInfo[res].iSubtitles = (int)(0.965 * 480);
    g_settings.m_ResInfo[res].iWidth = 720;
    g_settings.m_ResInfo[res].iHeight = 480;
    g_settings.m_ResInfo[res].dwFlags = NULL;//D3DPRESENTFLAG_INTERLACED | D3DPRESENTFLAG_WIDESCREEN;
    g_settings.m_ResInfo[res].fPixelRatio = 4320.0f / 4739.0f*4.0f / 3.0f;
    strcpy(g_settings.m_ResInfo[res].strMode, "PAL60 16:9");
    break;
  default:
    break;
  }
}

void CGraphicContext::SetRenderingResolution(RESOLUTION res, bool needsScaling)
{
	Lock();
	SetScalingResolution(res, needsScaling);
	UpdateCameraPosition(m_cameras.top());
	Unlock();
}

void CGraphicContext::SetScalingResolution(RESOLUTION res, bool needsScaling)
{
	m_windowResolution = res;

	if (needsScaling)
	{
		// Calculate necessary scalings
		float fFromWidth;
		float fFromHeight;
		float fToPosX;
		float fToPosY;
		float fToWidth;
		float fToHeight;

		{
			fFromWidth = (float)g_settings.m_ResInfo[res].iWidth;
			fFromHeight = (float)g_settings.m_ResInfo[res].iHeight;
			fToPosX = (float)g_settings.m_ResInfo[m_Resolution].Overscan.left;
			fToPosY = (float)g_settings.m_ResInfo[m_Resolution].Overscan.top;

			// FIXME: These values are all screwed up for some reason!
			//        Resolution us currently hardcoded so working around it for the moment!

			fToWidth = (float)g_settings.m_ResInfo[m_Resolution].Overscan.right - fToPosX; // FIXME
			fToHeight = (float)g_settings.m_ResInfo[m_Resolution].Overscan.bottom - fToPosY; // FIXME

			fToWidth = (float)g_settings.m_ResInfo[m_Resolution].Overscan.right + 15; // HACK!
			fToHeight = (float)g_settings.m_ResInfo[m_Resolution].Overscan.bottom - 130; // HACK!
		}

		// Add additional zoom to compensate for any overskan built in skin
		float fZoom = g_SkinInfo.GetSkinZoom();

		if(!g_guiSkinzoom) // Lookup gui setting if we didn't have it already
			g_guiSkinzoom = (CSettingInt*)g_guiSettings.GetSetting("lookandfeel.skinzoom");

		if(g_guiSkinzoom)
			fZoom *= (100 + g_guiSkinzoom->GetData()) * 0.01f;

		fZoom -= 1.0f;
		fToPosX -= fToWidth * fZoom * 0.5f;
		fToWidth *= fZoom + 1.0f;

		// Adjust for aspect ratio as zoom is given in the vertical direction and we don't
		// do aspect ratio corrections in the gui code
		fZoom = fZoom / g_settings.m_ResInfo[m_Resolution].fPixelRatio;
		fToPosY -= fToHeight * fZoom * 0.5f;
		fToHeight *= fZoom + 1.0f;

		m_guiScaleX = fFromWidth / fToWidth;
		m_guiScaleY = fFromHeight / fToHeight;
		TransformMatrix guiScaler = TransformMatrix::CreateScaler(fToWidth / fFromWidth, fToHeight / fFromHeight, fToHeight / fFromHeight);
		TransformMatrix guiOffset = TransformMatrix::CreateTranslation(fToPosX, fToPosY);
		m_guiTransform = guiOffset * guiScaler;
	}
	else
	{
		m_guiTransform.Reset();
		m_guiScaleX = 1.0f;
		m_guiScaleY = 1.0f;
	}

	// Reset our origin and camera
	while (m_origins.size())
		m_origins.pop();

	m_origins.push(CPoint(0, 0));

	while (m_cameras.size())
		m_cameras.pop();

	m_cameras.push(CPoint(0.5f*m_iScreenWidth, 0.5f*m_iScreenHeight));

	// and reset the final transform
	UpdateFinalTransform(m_guiTransform);
}

float CGraphicContext::GetScalingPixelRatio() const
{
	if(m_Resolution == m_windowResolution)
		return GetPixelRatio(m_windowResolution);

	RESOLUTION checkRes = m_windowResolution;
	if(checkRes == INVALID)
		checkRes = m_Resolution;
		
	// Resolutions are different - we want to return the aspect ratio of the video resolution
	// but only once it's been corrected for the skin -> screen coordinates scaling
	float winWidth = (float)g_settings.m_ResInfo[checkRes].iWidth;
	float winHeight = (float)g_settings.m_ResInfo[checkRes].iHeight;
	float outWidth = (float)g_settings.m_ResInfo[m_Resolution].iWidth;
	float outHeight = (float)g_settings.m_ResInfo[m_Resolution].iHeight;
	float outPR = GetPixelRatio(m_Resolution);

	return outPR * (outWidth / outHeight) / (winWidth / winHeight);
}

void CGraphicContext::SetCameraPosition(const CPoint &camera)
{
	// Offset the camera from our current location (this is in XML coordinates) and scale it up to
	// the screen resolution
	CPoint cam(camera);

	if (m_origins.size())
		cam += m_origins.top();

	RESOLUTION windowRes = (m_windowResolution == INVALID) ? m_Resolution : m_windowResolution;

	cam.x *= (float)m_iScreenWidth / g_settings.m_ResInfo[windowRes].iWidth;
	cam.y *= (float)m_iScreenHeight / g_settings.m_ResInfo[windowRes].iHeight;

	m_cameras.push(cam);

	Lock();
	UpdateCameraPosition(m_cameras.top());
	Unlock();
}

void CGraphicContext::RestoreCameraPosition()
{
	// Remove the top camera from the stack
	ASSERT(m_cameras.size());
	m_cameras.pop();

	Lock();
	UpdateCameraPosition(m_cameras.top());
    Unlock();
}

float CGraphicContext::GetPixelRatio(RESOLUTION iRes) const
{
	return g_settings.m_ResInfo[iRes].fPixelRatio;
}

void CGraphicContext::UpdateCameraPosition(const CPoint &camera)
{
	// NOTE: This routine is currently called (twice) every time there is a <camera>
	//       tag in the skin.  It actually only has to be called before we render
	//       something, so another option is to just save the camera coordinates
	//       and then have a routine called before every draw that checks whether
	//       the camera has changed, and if so, changes it.  Similarly, it could set
	//       the world transform at that point as well (or even combine world + view
	//       to cut down on one setting)

	// and calculate the offset from the screen center
	CPoint offset = camera - CPoint(m_iScreenWidth*0.5f, m_iScreenHeight*0.5f);

	// Brab the viewport dimensions and location
	D3DVIEWPORT9 viewport;
	g_graphicsContext.TLock();
	m_pd3dDevice->GetViewport(&viewport);
	g_graphicsContext.TUnlock();
	float w = viewport.Width*0.5f;
	float h = viewport.Height*0.5f;

	// World view. Until this is moved onto the GPU (via a vertex shader for instance), we set it to the identity
	// here.
	D3DXMATRIX mtxWorld;
	D3DXMatrixIdentity(&mtxWorld);
// m_pd3dDevice->SetTransform(D3DTS_WORLD, &mtxWorld);

	// Camera view.  Multiply the Y coord by -1 then translate so that everything is relative to the camera
	// position.
	D3DXMATRIX flipY, translate, mtxView;
	D3DXMatrixScaling(&flipY, 1.0f, -1.0f, 1.0f);
	D3DXMatrixTranslation(&translate, -(viewport.X + w + offset.x), -(viewport.Y + h + offset.y), 2*h);
	D3DXMatrixMultiply(&mtxView, &translate, &flipY);
//	m_pd3dDevice->SetTransform(D3DTS_VIEW, &mtxView);

	// Projection onto screen space
	D3DXMATRIX mtxProjection;
	D3DXMatrixPerspectiveOffCenterLH(&mtxProjection, (-w - offset.x)*0.5f, (w - offset.x)*0.5f, (-h + offset.y)*0.5f, (h + offset.y)*0.5f, h, 100*h);
 // m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mtxProjection);

	m_matFinal = mtxWorld * mtxView * mtxProjection;
}

bool CGraphicContext::RectIsAngled(float x1, float y1, float x2, float y2) const
{
	// Need only test 3 points, as they must be co-planer
	if (m_finalTransform.TransformZCoord(x1, y1, 0)) return true;
	if (m_finalTransform.TransformZCoord(x2, y2, 0)) return true;
	if (m_finalTransform.TransformZCoord(x1, y2, 0)) return true;

	return false;
}

void CGraphicContext::SetOrigin(float x, float y)
{
	if(m_origins.size())
		m_origins.push(CPoint(x,y) + m_origins.top());
	else
		m_origins.push(CPoint(x,y));
	
	AddTransform(TransformMatrix::CreateTranslation(x, y));
}

void CGraphicContext::RestoreOrigin()
{
	if(m_origins.size())
		m_origins.pop();
	
	RemoveTransform();
}

void CGraphicContext::EnablePreviewWindow(bool bEnable)
{
	m_bShowPreviewWindow = bEnable;
}

void CGraphicContext::Clear(DWORD color /*= 0x00010001*/)
{
	Lock();

	if (!m_pd3dDevice) return;

	//Not trying to clear the zbuffer when there is none is 7 fps faster (pal resolution)
	if ((!m_pd3dParams) || (m_pd3dParams->EnableAutoDepthStencil == TRUE))
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0L);
		g_graphicsContext.TUnlock();
	}
	else
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET, color, 1.0f, 0L);
		g_graphicsContext.TUnlock();
	}

	Unlock();
}

// Add a new clip region, intersecting with the previous clip region.
bool CGraphicContext::SetClipRegion(float x, float y, float w, float h)
{
	// Transform from our origin
	CPoint origin;
	
	if (m_origins.size())
		origin = m_origins.top();
	
	// ok, now intersect with our old clip region
	CRect rect(x, y, x + w, y + h);
	rect += origin;
	
	if (m_clipRegions.size())
	{
		// Intersect with original clip region
		rect.Intersect(m_clipRegions.top());
	}
	
	if (rect.IsEmpty())
		return false;
	
	m_clipRegions.push(rect);

	// here we could set the hardware clipping, if applicable
	return true;
}

void CGraphicContext::RestoreClipRegion()
{
	if (m_clipRegions.size())
		m_clipRegions.pop();

	// here we could reset the hardware clipping, if applicable
}

void CGraphicContext::ClipRect(CRect &vertex, CRect &texture, CRect *texture2)
{
	// This is the software clipping routine.  If the graphics hardware is set to do the clipping
	// (eg via SetClipPlane in D3D for instance) then this routine is unneeded.
	if (m_clipRegions.size())
	{
		// Take a copy of the vertex rectangle and intersect
		// it with our clip region (moved to the same coordinate system)
		CRect clipRegion(m_clipRegions.top());
		
		if (m_origins.size())
			clipRegion -= m_origins.top();
		
		CRect original(vertex);
		vertex.Intersect(clipRegion);
		
		// and use the original to compute the texture coordinates
		if (original != vertex)
		{
			float scaleX = texture.Width() / original.Width();
			float scaleY = texture.Height() / original.Height();
			texture.x1 += (vertex.x1 - original.x1) * scaleX;
			texture.y1 += (vertex.y1 - original.y1) * scaleY;
			texture.x2 += (vertex.x2 - original.x2) * scaleX;
			texture.y2 += (vertex.y2 - original.y2) * scaleY;
			
			if (texture2)
			{
				scaleX = texture2->Width() / original.Width();
				scaleY = texture2->Height() / original.Height();
				texture2->x1 += (vertex.x1 - original.x1) * scaleX;
				texture2->y1 += (vertex.y1 - original.y1) * scaleY;
				texture2->x2 += (vertex.x2 - original.x2) * scaleX;
				texture2->y2 += (vertex.y2 - original.y2) * scaleY;
			}
		}
	}
}

void CGraphicContext::UpdateFinalTransform(const TransformMatrix &matrix)
{
	m_finalTransform = matrix;
	// We could set the world transform here to GPU-ize the animation system.
	// trouble is that we require the resulting x,y coords to be rounded to
	// the nearest pixel (vertex shader perhaps?)
}
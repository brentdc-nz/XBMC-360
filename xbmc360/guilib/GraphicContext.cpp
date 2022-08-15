/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GraphicContext.h"
#include "utils\log.h"
#include "Settings.h"
#include "GuiSettings.h"

CGraphicContext g_graphicsContext;

// Quick access to a skin setting, fine unless we starts clearing video settings
static CSettingInt* g_guiSkinzoom = NULL;

D3DXMATRIX matWVPn; //FIXME: Remove from being global and create a "get" function

CGraphicContext::CGraphicContext(void)
{
	m_pd3dDevice = NULL;
	m_XUIDevice = NULL;

	m_bFullScreenVideo = false;
	m_bShowPreviewWindow = false;

	m_iScreenWidth = 0;
	m_iScreenHeight = 0;
	m_strMediaDir = "";

	m_Resolution = INVALID;
	m_guiScaleX = m_guiScaleY = 1.0f;
	m_windowResolution = INVALID;
}

CGraphicContext::~CGraphicContext(void)
{
}

void CGraphicContext::SetD3DDevice(LPDIRECT3DDEVICE9 p3dDevice)
{
	m_pd3dDevice = p3dDevice;

	// Release the object for any thread to acquire!
	m_pd3dDevice->ReleaseThreadOwnership();
}

void CGraphicContext::SetXUIDevice(HXUIDC pXUIDevice)
{
	m_XUIDevice = pXUIDevice;
}

void CGraphicContext::SetD3DParameters(D3DPRESENT_PARAMETERS *p3dParams)
{
	m_pd3dParams = p3dParams;

    m_iScreenWidth = m_pd3dParams->BackBufferWidth; //MARTY FIXME Move to correct place
    m_iScreenHeight = m_pd3dParams->BackBufferHeight; //MARTY FIXME Move to correct place
}

void CGraphicContext::SetVideoResolution(RESOLUTION &res, BOOL NeedZ, bool forceClear /* = false */)
{
	if(res == AUTORES)
	{
//		res = g_videoConfig.GetBestMode(); //TODO
	}

/*
	if(!IsValidResolution(res)) //TODO
	{
		// Choose a failsafe resolution that we can actually display
		CLog::Log(LOGERROR, "The screen resolution requested is not valid, resetting to a valid mode");
		res = g_videoConfig.GetSafeMode();
	}
*/
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

		if(res == HDTV_1080i || res == HDTV_720p || m_bFullScreenVideo)
			m_pd3dParams->BackBufferCount = 1;
		else
			m_pd3dParams->BackBufferCount = 2;
/*
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
*/	}

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
			m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00010001, 1.0f, 0L );
			m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
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
//		if(m_Resolution != INVALID)
//			g_fontManager.ReloadTTFFonts();
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
		case HDTV_1080i:
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

const RECT& CGraphicContext::GetViewWindow() const
{
	return m_videoRect;
}

void CGraphicContext::SetViewWindow(const RECT& rc)
{
	Lock();
	m_videoRect.left = rc.left;//(long)(ScaleFinalXCoord((float)rc.left, (float)rc.top) + 0.5f);
    m_videoRect.top = rc.top;//(long)(ScaleFinalYCoord((float)rc.left, (float)rc.top) + 0.5f);
    m_videoRect.right = rc.right;//(long)(ScaleFinalXCoord((float)rc.right, (float)rc.bottom) + 0.5f);
    m_videoRect.bottom = rc.bottom;//(long)(ScaleFinalYCoord((float)rc.right, (float)rc.bottom) + 0.5f);
    if (m_bShowPreviewWindow && !m_bFullScreenVideo)
    {
		D3DRECT d3dRC;
		d3dRC.x1 = m_videoRect.left;
		d3dRC.x2 = m_videoRect.right;
		d3dRC.y1 = m_videoRect.top;
		d3dRC.y2 = m_videoRect.bottom;
		Get3DDevice()->Clear( 1, &d3dRC, D3DCLEAR_TARGET, 0x00010001, 1.0f, 0L );
    }
	Unlock();
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

	m_pd3dDevice->AcquireThreadOwnership();
}

void CGraphicContext::ResetScreenParameters(RESOLUTION res)
{

  // 1080i
  switch (res)
  {
  case HDTV_1080i:
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

// Overloaded for debugging 
void CGraphicContext::Lock(std::string strFunction)
{
	std::string strTmp = "GFX LOCK CALLER = ";
	strTmp += strFunction += "\n";
///	OutputDebugString(strTmp.c_str());

	if(!m_pd3dDevice)
		return;

	EnterCriticalSection(*this);

	m_pd3dDevice->AcquireThreadOwnership();
}

void CGraphicContext::Unlock()
{
	if(!m_pd3dDevice)
		return;
	
	m_pd3dDevice->ReleaseThreadOwnership();

	LeaveCriticalSection(*this);
}

// Overloaded for debugging 
void CGraphicContext::Unlock(std::string strFunction)
{
	std::string strTmp = "GFX UN-LOCK CALLER = ";
	strTmp += strFunction += "\n";
///	OutputDebugString(strTmp.c_str());

	if(!m_pd3dDevice)
		return;
	
	m_pd3dDevice->ReleaseThreadOwnership();

	LeaveCriticalSection(*this);
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

	if(needsScaling)
	{
		// calculate necessary scalings
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
			fToWidth = (float)g_settings.m_ResInfo[m_Resolution].Overscan.right - fToPosX;
			fToHeight = (float)g_settings.m_ResInfo[m_Resolution].Overscan.bottom - fToPosY;
		}

		// Add additional zoom to compensate for any overskan built in skin
		float fZoom = 1;//g_SkinInfo.GetSkinZoom();

		if(!g_guiSkinzoom) // lookup gui setting if we didn't have it already
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
	while(m_origins.size())
		m_origins.pop();

	m_origins.push(CPoint(0, 0));

	while(m_cameras.size())
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
	m_pd3dDevice->GetViewport(&viewport);
	float w = viewport.Width*0.5f;
	float h = viewport.Height*0.5f;

	// World view.  Until this is moved onto the GPU (via a vertex shader for instance), we set it to the identity
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

	matWVPn = mtxWorld * mtxView * mtxProjection;
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
		m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0L);
	else
		m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET, color, 1.0f, 0L);

	Unlock();
}


// add a new clip region, intersecting with the previous clip region.
bool CGraphicContext::SetClipRegion(float x, float y, float w, float h)
{ // transform from our origin
  CPoint origin;
  if (m_origins.size())
    origin = m_origins.top();
  // ok, now intersect with our old clip region
  CRect rect(x, y, x + w, y + h);
  rect += origin;
  if (m_clipRegions.size())
  { // intersect with original clip region
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
  // this is the software clipping routine.  If the graphics hardware is set to do the clipping
  // (eg via SetClipPlane in D3D for instance) then this routine is unneeded.
  if (m_clipRegions.size())
  {
    // take a copy of the vertex rectangle and intersect
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


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

CGraphicContext g_graphicsContext;

CGraphicContext::CGraphicContext(void)
{
	m_pd3dDevice = NULL;
	m_XUIDevice = NULL;

	m_bFullScreenVideo = false;
	m_bShowPreviewWindow = false;

	m_iScreenWidth = 0;
	m_iScreenHeight = 0;
	m_strMediaDir = "";
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
	m_strMediaDir = strMediaDir + "\\";
	g_TextureManager.SetTexturePath(m_strMediaDir);
}

void CGraphicContext::Lock()
{
	if(!m_pd3dDevice)
		return;

	EnterCriticalSection(*this);

	m_pd3dDevice->AcquireThreadOwnership();
}

// Overloaded for debugging 
void CGraphicContext::Lock(std::string strFunction)
{
	std::string strTmp = "GFX LOCK CALLER = ";
	strTmp += strFunction += "\n";
	OutputDebugString(strTmp.c_str());

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
	OutputDebugString(strTmp.c_str());

	if(!m_pd3dDevice)
		return;
	
	m_pd3dDevice->ReleaseThreadOwnership();

	LeaveCriticalSection(*this);
}

void CGraphicContext::EnablePreviewWindow(bool bEnable)
{
	m_bShowPreviewWindow = bEnable;
}

void CGraphicContext::Clear(DWORD color)
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
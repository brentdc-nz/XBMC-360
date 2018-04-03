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

#include "Splash.h"
#include "Log.h"
#include "..\guilib\GUIImage.h"
#include "..\filesystem\File.h"
#include "..\guilib\GUIButtonControl.h"

using namespace XFILE;

CSplash::CSplash(const CStdString& imageName)
{
	m_ImageName = imageName;
}

CSplash::~CSplash()
{
	Stop();
}

void CSplash::OnStartup()
{
}

void CSplash::OnExit()
{
}

void CSplash::Process()
{
	D3DGAMMARAMP newRamp;
	D3DGAMMARAMP oldRamp;

	g_graphicsContext.Lock();
	g_graphicsContext.Get3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
	g_graphicsContext.Unlock();

	float w = g_graphicsContext.GetWidth() * 0.5f;
	float h = g_graphicsContext.GetHeight() * 0.5f;
	
	CGUIImage* image = new CGUIImage(0, 0, w*0.5f, h*0.5f, w, h, m_ImageName);
	image->AllocResources();

	// Store the old gamma ramp
	g_graphicsContext.Lock();
	g_graphicsContext.Get3DDevice()->GetGammaRamp(NULL, &oldRamp);
	g_graphicsContext.Unlock();

	float fade = 0.5f;
	for (int i = 0; i < 256; i++)
	{
		newRamp.red[i] = (int)((float)oldRamp.red[i] * fade);
		newRamp.green[i] = (int)((float)oldRamp.red[i] * fade);
		newRamp.blue[i] = (int)((float)oldRamp.red[i] * fade);
	}
	
	g_graphicsContext.Lock();
	g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, D3DSGR_IMMEDIATE, &newRamp);
	
	// Render splash image
	g_graphicsContext.Get3DDevice()->BeginScene();

	// FIXME - Why not alpha blended like all other CGUIImage instances!?
	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x0000008f);
	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	g_graphicsContext.Unlock();

	image->Render();
	image->FreeResources();
	delete image;

	// Show it on screen
	g_graphicsContext.Lock();

	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_ALPHAREF, NULL);
	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); 
	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_ALPHAFUNC,  NULL);

	g_graphicsContext.Get3DDevice()->EndScene();

	g_graphicsContext.Get3DDevice()->Present( NULL, NULL, NULL, NULL );
	g_graphicsContext.Unlock();

	// Fade in and wait untill the thread is stopped
	while (!m_bStop)
	{
		if (fade <= 1.f)
		{
			Sleep(1);
			for (int i = 0; i < 256; i++)
			{
				newRamp.red[i] = (int)((float)oldRamp.red[i] * fade);
				newRamp.green[i] = (int)((float)oldRamp.green[i] * fade);
				newRamp.blue[i] = (int)((float)oldRamp.blue[i] * fade);
			}
			g_graphicsContext.Lock();
			g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, D3DSGR_IMMEDIATE, &newRamp);
			g_graphicsContext.Unlock();
			fade += 0.01f;
		}
		else
		{
			Sleep(10);
		}
	}

	g_graphicsContext.Lock();
	
	// Fade out
	for (float fadeout = fade - 0.01f; fadeout >= 0.f; fadeout -= 0.01f)
	{
		for (int i = 0; i < 256; i++)
		{
			newRamp.red[i] = (int)((float)oldRamp.red[i] * fadeout);
			newRamp.green[i] = (int)((float)oldRamp.green[i] * fadeout);
			newRamp.blue[i] = (int)((float)oldRamp.blue[i] * fadeout);
		}
		Sleep(1);
		g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, D3DSGR_IMMEDIATE, &newRamp);
	}
	
	// Restore original gamma ramp
	g_graphicsContext.Get3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
	g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, D3DSGR_IMMEDIATE, &oldRamp);
	g_graphicsContext.Get3DDevice()->Present( NULL, NULL, NULL, NULL );
	g_graphicsContext.Unlock();
}

bool CSplash::Start()
{
	if (m_ImageName.IsEmpty() || !CFile::Exists(m_ImageName))
	{
		CLog::Log(LOGDEBUG, "Splash image %s not found", m_ImageName.c_str());
		return false;
	}
	Create();
	return true;
}

void CSplash::Stop()
{
	StopThread();
}

bool CSplash::IsRunning()
{
	return (m_ThreadHandle != NULL);
}
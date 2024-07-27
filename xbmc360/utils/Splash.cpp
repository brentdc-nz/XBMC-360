#include "Splash.h"
#include "Log.h"
#include "guilib\GUIImage.h"
#include "filesystem\File.h"
#include "guilib\GUIButtonControl.h"

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

	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
	g_graphicsContext.TUnlock();
  
	g_graphicsContext.SetCameraPosition(CPoint(0, 0));
	float w = g_graphicsContext.GetWidth() * 0.5f;
	float h = g_graphicsContext.GetHeight() * 0.5f;
	
	CGUIImage* image = new CGUIImage(0, 0, w*0.5f, h*0.5f, w, h, m_ImageName);
	image->SetAspectRatio(CAspectRatio::AR_KEEP);
	image->AllocResources();

	// Store the old gamma ramp
	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->GetGammaRamp(NULL, &oldRamp);
	g_graphicsContext.TUnlock();

	float fade = 0.5f;
	for (int i = 0; i < 256; i++)
	{
		newRamp.red[i] = (int)((float)oldRamp.red[i] * fade);
		newRamp.green[i] = (int)((float)oldRamp.red[i] * fade);
		newRamp.blue[i] = (int)((float)oldRamp.red[i] * fade);
	}

	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, NULL, &newRamp);
	g_graphicsContext.TUnlock();

	// Render splash image
	image->Render();
	image->FreeResources();
	delete image;
	
	// Show it on screen
	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->Present( NULL, NULL, NULL, NULL );
	g_graphicsContext.TUnlock();

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
			g_graphicsContext.TLock();
			g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, NULL, &newRamp);
			g_graphicsContext.TUnlock();
			fade += 0.01f;
		}
		else
		{
			Sleep(10);
		}
	}

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
		g_graphicsContext.TLock();
		g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, NULL, &newRamp);
		g_graphicsContext.TUnlock();
	}

	// Restore original gamma ramp
	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
	g_graphicsContext.Get3DDevice()->SetGammaRamp(NULL, NULL, &oldRamp);
	g_graphicsContext.Get3DDevice()->Present( NULL, NULL, NULL, NULL );
	g_graphicsContext.TUnlock();
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
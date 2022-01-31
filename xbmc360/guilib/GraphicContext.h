#ifndef GUILIB_GRAPHICCONTEXT_H
#define GUILIB_GRAPHICCONTEXT_H

#include "..\utils\Stdafx.h"
#include "..\utils\StdString.h"
#include "..\utils\CriticalSection.h"  // base class
#include "TextureManager.h"

#include <xui.h>

// Output the function name in debug mode
#ifdef _DEBUG
#define GRAPHICSCONTEXT_LOCK()		g_graphicsContext.Lock(__FUNCTION__);
#define GRAPHICSCONTEXT_UNLOCK()	g_graphicsContext.Unlock(__FUNCTION__);
#else
#define GRAPHICSCONTEXT_LOCK()		g_graphicsContext.Lock();
#define GRAPHICSCONTEXT_UNLOCK()	g_graphicsContext.Unlock();
#endif

class CGraphicContext : public CCriticalSection
{
public:
	CGraphicContext(void);
	virtual ~CGraphicContext(void);
	LPDIRECT3DDEVICE9 Get3DDevice() { return m_pd3dDevice; }
	HXUIDC GetXUIDevice() { return m_XUIDevice; }
	void SetD3DDevice(LPDIRECT3DDEVICE9 p3dDevice);
	void SetXUIDevice(HXUIDC pXUIDevice);
	void SetD3DParameters(D3DPRESENT_PARAMETERS *p3dParams);

	int GetWidth() const { return m_iScreenWidth; }
	int GetHeight() const { return m_iScreenHeight; }
	const CStdString& GetMediaDir() const { return m_strMediaDir; }
	void SetMediaDir(const CStdString &strMediaDir);

	const RECT& GetViewWindow() const;
	void SetViewWindow(const RECT& rc);

	void SetFullScreenVideo(bool bOnOff);
	bool IsFullScreenVideo() const;

	void Lock(std::string strFunction);
	void Unlock(std::string strFunction);
	void Lock();
	void Unlock();

	void EnablePreviewWindow(bool bEnable);

	void Clear(DWORD color = 0x00010001);

private:
	HXUIDC					m_XUIDevice;

	LPDIRECT3DDEVICE9		m_pd3dDevice;
	D3DPRESENT_PARAMETERS*	m_pd3dParams;

	int m_iScreenHeight;
	int m_iScreenWidth;
	CStdString m_strMediaDir;

	RECT m_videoRect;
	bool m_bFullScreenVideo;
	bool m_bShowPreviewWindow;
};

extern CGraphicContext g_graphicsContext;

#endif //GUILIB_GRAPHICCONTEXT_H
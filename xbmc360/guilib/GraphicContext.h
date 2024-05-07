#ifndef GUILIB_GRAPHICCONTEXT_H
#define GUILIB_GRAPHICCONTEXT_H

#include "utils\Stdafx.h"
#include "utils\StdString.h"
#include "utils\CriticalSection.h" // Base class
#include "TransformMatrix.h" // For the members m_guiTransform etc
#include "TextureManager.h"
#include "Geometry.h"

#include <stack>

enum RESOLUTION
{
	INVALID = -1,
	HDTV_1080p = 0,
	HDTV_720p = 1,
	HDTV_480p_4x3 = 2,
	HDTV_480p_16x9 = 3,
	NTSC_4x3 = 4,
	NTSC_16x9 = 5,
	PAL_4x3 = 6,
	PAL_16x9 = 7,
	PAL60_4x3 = 8,
	PAL60_16x9 = 9,
	AUTORES = 10
};

enum VIEW_TYPE
{ 
	VIEW_TYPE_NONE = 0,
	VIEW_TYPE_LIST,
	VIEW_TYPE_ICON,
	VIEW_TYPE_BIG_LIST,
	VIEW_TYPE_BIG_ICON,
	VIEW_TYPE_WIDE,
	VIEW_TYPE_BIG_WIDE,
	VIEW_TYPE_WRAP,
	VIEW_TYPE_BIG_WRAP,
	VIEW_TYPE_AUTO,
	VIEW_TYPE_MAX
};

struct OVERSCAN
{
	int left;
	int top;
	int right;
	int bottom;
};

struct RESOLUTION_INFO
{
	OVERSCAN Overscan;
	int iWidth;
	int iHeight;
	int iSubtitles;
	DWORD dwFlags;
	float fPixelRatio;
	char strMode[11];
};

class CGraphicContext : public CCriticalSection
{
public:
	CGraphicContext(void);
	virtual ~CGraphicContext(void);

	LPDIRECT3DDEVICE9 Get3DDevice() { return m_pd3dDevice; }
	void SetD3DDevice(LPDIRECT3DDEVICE9 p3dDevice);
	void SetD3DParameters(D3DPRESENT_PARAMETERS *p3dParams);
	void SetVideoResolution(RESOLUTION &res, BOOL NeedZ = FALSE, bool forceClear = false);
	RESOLUTION GetVideoResolution() const;
	void ResetOverscan(RESOLUTION res, OVERSCAN &overscan);
	int GetWidth() const { return m_iScreenWidth; }
	int GetHeight() const { return m_iScreenHeight; }
	int GetFPS() const;
	const CStdString& GetMediaDir() const { return m_strMediaDir; }
	void SetMediaDir(const CStdString &strMediaDir);
	const RECT& GetViewWindow() const;
	void SetViewWindow(float left, float top, float right, float bottom);
	bool SetViewPort(float fx, float fy , float fwidth, float fheight, bool intersectPrevious = false);
	void RestoreViewPort();
	void SetFullScreenVideo(bool bOnOff);
	bool IsFullScreenVideo() const;
	void ResetScreenParameters(RESOLUTION res);
	float GetPixelRatio(RESOLUTION iRes) const;

	void Lock();
	void Unlock();

// TEST START
	__forceinline void TLock();
	__forceinline void TUnlock();
// TEST END

	// Output scaling
	void SetRenderingResolution(RESOLUTION res, bool needsScaling);  // Sets scaling up for rendering
	void SetScalingResolution(RESOLUTION res, bool needsScaling);    // Sets scaling up for skin loading etc
	float GetScalingPixelRatio() const;

	inline float ScaleFinalXCoord(float x, float y) const { return m_finalTransform.TransformXCoord(x, y, 0); }
	inline float ScaleFinalYCoord(float x, float y) const { return m_finalTransform.TransformYCoord(x, y, 0); }
	inline float ScaleFinalZCoord(float x, float y) const { return m_finalTransform.TransformZCoord(x, y, 0); }
	inline void ScaleFinalCoords(float &x, float &y, float &z) const { m_finalTransform.TransformPosition(x, y, z); }
	bool RectIsAngled(float x1, float y1, float x2, float y2) const;

	inline float GetGUIScaleX() const { return m_guiScaleX; };
	inline float GetGUIScaleY() const { return m_guiScaleY; };

	inline DWORD MergeAlpha(color_t color) const
	{
		color_t alpha = m_finalTransform.TransformAlpha((color >> 24) & 0xff);

		if (alpha > 255) alpha = 255;
			return ((alpha << 24) & 0xff000000) | (color & 0xffffff);
	}

	void SetOrigin(float x, float y);
	void RestoreOrigin();

	void SetCameraPosition(const CPoint &camera);
	void RestoreCameraPosition();

	void EnablePreviewWindow(bool bEnable);

	void Clear(DWORD color = 0x00010001);

	bool SetClipRegion(float x, float y, float w, float h);
	void RestoreClipRegion();
	void ClipRect(CRect &vertex, CRect &texture, CRect *diffuse = NULL);

	inline void ResetWindowTransform()
	{
		while(m_groupTransform.size())
			m_groupTransform.pop();

		m_groupTransform.push(m_guiTransform);
	}

	inline void AddTransform(const TransformMatrix &matrix)
	{
		ASSERT(m_groupTransform.size());

		if(m_groupTransform.size())
			m_groupTransform.push(m_groupTransform.top() * matrix);
		else
			m_groupTransform.push(matrix);

		UpdateFinalTransform(m_groupTransform.top());
	}

	inline void RemoveTransform()
	{
		ASSERT(m_groupTransform.size() > 1);

		if(m_groupTransform.size())
			m_groupTransform.pop();

		if(m_groupTransform.size())
			UpdateFinalTransform(m_groupTransform.top());
		else
			UpdateFinalTransform(TransformMatrix());
	}

	D3DXMATRIX GetFinalMatrix() { return m_matFinal; }

protected:
	LPDIRECT3DDEVICE9		m_pd3dDevice;
	D3DPRESENT_PARAMETERS*	m_pd3dParams;
	std::stack<D3DVIEWPORT9*> m_viewStack;
	int m_iScreenHeight;
	int m_iScreenWidth;
	CStdString m_strMediaDir;
	RECT m_videoRect;
	bool m_bFullScreenVideo;
	bool m_bShowPreviewWindow;
	RESOLUTION m_Resolution;

private:
	void UpdateCameraPosition(const CPoint &camera);
	void UpdateFinalTransform(const TransformMatrix &matrix);
	RESOLUTION m_windowResolution;
	float m_guiScaleX;
	float m_guiScaleY;
	std::stack<CPoint> m_cameras;
	std::stack<CPoint> m_origins;
	std::stack<CRect>  m_clipRegions;

	TransformMatrix m_guiTransform;
	TransformMatrix m_finalTransform;
	std::stack<TransformMatrix> m_groupTransform;

	D3DXMATRIX m_matFinal;
};

extern CGraphicContext g_graphicsContext;

#endif //GUILIB_GRAPHICCONTEXT_H
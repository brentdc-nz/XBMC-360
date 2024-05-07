#ifndef CRENDERMANAGER_H
#define CRENDERMANAGER_H

#include "BaseRenderer.h"
#include "RGBRenderer.h"

#include "utils\SharedSection.h"

#define CONF_FLAGS_FULLSCREEN    0x10

enum EFIELDSYNC
{
	FS_NONE,
	FS_ODD,
	FS_EVEN,
	FS_BOTH
};

class CRenderManager
{
public:
	CRenderManager();
	~CRenderManager();

	void Update(bool bPauseDrawing);
	void RenderUpdate(bool clear, DWORD flags = 0, DWORD alpha = 255);
	bool PreInit();
	bool Configure(int width, int height, unsigned flags);
	bool IsConfigured();
	bool GetImage(YV12Image *image);
	void ReleaseImage();
	void PrepareDisplay();
	void FlipPage(DWORD timestamp = 0L, int source = -1, EFIELDSYNC sync = FS_NONE);

	void UnInit();
	float GetMaximumFPS() { return 60.0; };
	inline bool IsStarted() { return m_bIsStarted; }

protected:
	CBaseRenderer* m_pRenderer; // Current Renderer

	bool m_bPauseDrawing; // True if we should pause rendering
	DWORD m_dwPresentTime;
	bool m_bIsStarted;
	CSharedSection m_sharedSection;
};

extern CRenderManager g_renderManager;

#endif //CRENDERMANAGER_H

#ifndef CRENDERMANAGER_H
#define CRENDERMANAGER_H

#include "BaseRenderer.h"
#include "RGBRenderer.h"

#include "..\..\utils\SharedSection.h"

class CRenderManager
{
public:
	CRenderManager();
	~CRenderManager();

	void RenderUpdate(bool clear, DWORD flags = 0, DWORD alpha = 255);
	bool PreInit();
	bool Configure(int width, int height);
	bool GetImage(RGB32Image_t *image);
	void ReleaseImage();
	void PrepareDisplay();
	void FlipPage();

	void UnInit();

	inline bool IsStarted() { return m_bIsStarted;}

protected:
	CBaseRenderer* m_pRenderer;//Current Renderer

	bool m_bIsStarted;
	CSharedSection m_sharedSection;
};

extern CRenderManager g_renderManager;

#endif //CRENDERMANAGER_H

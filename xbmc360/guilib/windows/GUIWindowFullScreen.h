#ifndef GUILIB_GUIWINDOWFULLSCREEN_H
#define GUILIB_GUIWINDOWFULLSCREEN_H

#include "..\GUIWindow.h"

class CGUIWindowFullScreen : public CGUIWindow
{
public:
	CGUIWindowFullScreen(void);
	virtual ~CGUIWindowFullScreen(void);

	virtual void AllocResources(bool forceLoad = false );
	virtual void FreeResources(bool forceUnload = false);

	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);

	virtual void Render();
	virtual void OnWindowLoaded();

	void RenderFullScreen();
	bool NeedRenderFullScreen();

private:
	void PreloadDialog(unsigned int windowID);
	void UnloadDialog(unsigned int windowID);

	bool m_bShowCurrentTime;
	bool m_bLastRender;
};

#endif //GUILIB_GUIWINDOWFULLSCREEN_H
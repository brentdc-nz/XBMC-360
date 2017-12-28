#ifndef GUILIB_GUIWINDOWFULLSCREEN_H
#define GUILIB_GUIWINDOWFULLSCREEN_H

#include "..\GUIWindow.h"

class CGUIWindowFullScreen : public CGUIWindow
{
public:
	CGUIWindowFullScreen(void);
	virtual ~CGUIWindowFullScreen(void);

	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);

	virtual void Render();

	void RenderFullScreen();
	bool NeedRenderFullScreen();
};

#endif //GUILIB_GUIWINDOWFULLSCREEN_H
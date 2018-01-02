#ifndef GUILIB_GUIWINDOWSCREENSAVER_H
#define GUILIB_GUIWINDOWSCREENSAVER_H

#include "..\GUIWindow.h"
#include "..\screensavers\ScreensaverBase.h"

class CGUIWindowScreensaver : public CGUIWindow
{
public:
	CGUIWindowScreensaver(void);
	virtual ~CGUIWindowScreensaver(void);

	virtual void Render();
	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);

private:
	CSceensaverBase* m_pScreensaver;
};

#endif //GUILIB_GUIWINDOWSCREENSAVER_H

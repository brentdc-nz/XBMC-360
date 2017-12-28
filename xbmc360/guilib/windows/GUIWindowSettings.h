#ifndef GUILIB_GUIWINDOWSETTINGS_H
#define GUILIB_GUIWINDOWSETTINGS_H

#include "..\GUIWindow.h"

class CGUIWindowSettimgs : public CGUIWindow
{
public:
	CGUIWindowSettimgs(void);
	virtual ~CGUIWindowSettimgs(void);

	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);

private:
};

#endif //GUILIB_GUIWINDOWSETTINGS_H

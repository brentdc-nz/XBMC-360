#ifndef GUILIB_GUIWINDOWSETTINGS_H
#define GUILIB_GUIWINDOWSETTINGS_H

#include "..\GUIWindow.h"

class CGUIWindowSettings : public CGUIWindow
{
public:
	CGUIWindowSettings(void);
	virtual ~CGUIWindowSettings(void);

	virtual bool OnMessage(CGUIMessage& message);
private:
};

#endif //GUILIB_GUIWINDOWSETTINGS_H

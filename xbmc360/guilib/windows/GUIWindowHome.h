#ifndef GUILIB_GUIWINDOWHOME_H
#define GUILIB_GUIWINDOWHOME_H

#include "..\GUIWindow.h"

class CGUIWindowHome : public CGUIWindow
{
public:
	CGUIWindowHome(void);
	virtual ~CGUIWindowHome(void);

	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);

private:
};

#endif //GUILIB_GUIWINDOWHOME_H

#ifndef GUILIB_GUIWINDOWVIDEOFILES_H
#define GUILIB_GUIWINDOWVIDEOFILES_H

#include "..\GUIWindow.h"

class CGUIWindowVideoFiles : public CGUIWindow
{
public:
	CGUIWindowVideoFiles(void);
	virtual ~CGUIWindowVideoFiles(void);

	virtual bool OnMessage(CGUIMessage& message);
};

#endif //GUILIB_GUIWINDOWVIDEOFILES_H

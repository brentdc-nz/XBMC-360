#ifndef GUILIB_GUIWINDOWVIDEOFILES_H
#define GUILIB_GUIWINDOWVIDEOFILES_H

#include "..\GUIWindow.h"
#include "..\..\FileItem.h"

class CGUIWindowVideoFiles : public CGUIWindow
{
public:
	CGUIWindowVideoFiles(void);
	virtual ~CGUIWindowVideoFiles(void);

	virtual bool OnMessage(CGUIMessage& message);
private:

	CFileItemList m_items; //TEST
};

#endif //GUILIB_GUIWINDOWVIDEOFILES_H

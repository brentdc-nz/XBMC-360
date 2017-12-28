#ifndef GUILIB_GUIWINDOWSETTIMGSCATEGORY_H
#define GUILIB_GUIWINDOWSETTIMGSCATEGORY_H

#include "..\GUIWindow.h"

class CGUIWindowSettimgsCategory : public CGUIWindow
{
public:
	CGUIWindowSettimgsCategory(void);
	virtual ~CGUIWindowSettimgsCategory(void);

	virtual bool OnMessage(CGUIMessage& message);

private:
	int m_iScreen;
};

#endif //GUILIB_GUIWINDOWSETTIMGSCATEGORY_H

#ifndef GUILIB_GUIWINDOWVIDEOFILES_H
#define GUILIB_GUIWINDOWVIDEOFILES_H

#include "..\GUIWindow.h"
#include "..\GUIMediaWindow.h"
#include "..\..\ThumbLoader.h"

class CGUIWindowVideoFiles : public CGUIMediaWindow
{
public:
	CGUIWindowVideoFiles(void);
	virtual ~CGUIWindowVideoFiles(void);

	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnClick(int iItem);
	bool Update(const CStdString &strDirectory);

private:
	CVideoThumbLoader m_thumbLoader;
};

#endif //GUILIB_GUIWINDOWVIDEOFILES_H

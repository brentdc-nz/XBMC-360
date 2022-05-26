#ifndef GUILIB_GUIWINDOWPICTURES_H
#define GUILIB_GUIWINDOWPICTURES_H

#include "..\GUIWindow.h"
#include "..\GUIMediaWindow.h"
#include "..\..\ThumbLoader.h"

class CGUIWindowPictures : public CGUIMediaWindow
{
public:
	CGUIWindowPictures(void);
	virtual ~CGUIWindowPictures(void);

	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnClick(int iItem);
	bool Update(const CStdString &strDirectory);
	bool OnPlayMedia(int iItem);

private:
	CPicturesThumbLoader m_thumbLoader;
};

#endif //GUILIB_GUIWINDOWPICTURES_H
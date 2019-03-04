#ifndef GUILIB_GUIWINDOWMUSICFILES_H
#define GUILIB_GUIWINDOWMUSICFILES_H

#include "..\GUIWindow.h"
#include "..\GUIMediaWindow.h"
#include "..\..\ThumbLoader.h"

class CGUIWindowMusicFiles : public CGUIMediaWindow
{
public:
	CGUIWindowMusicFiles(void);
	virtual ~CGUIWindowMusicFiles(void);

	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnClick(int iItem);
	bool Update(const CStdString &strDirectory);

private:
	CMusicThumbLoader m_thumbLoader;
};

#endif //GUILIB_GUIWINDOWMUSICFILES_H

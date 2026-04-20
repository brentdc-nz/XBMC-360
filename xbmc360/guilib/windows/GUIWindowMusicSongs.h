#ifndef GUILIB_GUIWINDOWMUSICSONGS_H
#define GUILIB_GUIWINDOWMUSICSONGS_H

#include "guilib\GUIMediaWindow.h"
#include "ThumbLoader.h"

class CGUIWindowMusicSongs : public CGUIMediaWindow
{
public:
	CGUIWindowMusicSongs(void);
	virtual ~CGUIWindowMusicSongs(void);
	
	virtual bool OnMessage(CGUIMessage& message);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
	virtual bool Update(const CStdString &strDirectory);

	CMusicThumbLoader m_thumbLoader;
};

#endif //GUILIB_GUIWINDOWMUSICSONGS_H

#ifndef GUILIB_GUIWINDOWMUSICSONGS_H
#define GUILIB_GUIWINDOWMUSICSONGS_H

#include "guilib\GUIMediaWindow.h"

class CGUIWindowMusicSongs : public CGUIMediaWindow
{
public:
	CGUIWindowMusicSongs(void);
	virtual ~CGUIWindowMusicSongs(void);
	
	virtual bool OnMessage(CGUIMessage& message);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
};

#endif //GUILIB_GUIWINDOWMUSICSONGS_H

#ifndef GUILIB_GUIWINDOWMUSIC_H
#define GUILIB_GUIWINDOWMUSIC_H

#include "guilib\GUIMediaWindow.h"

class CGUIWindowMusicFiles : public CGUIMediaWindow
{
public:
	CGUIWindowMusicFiles(void);
	virtual ~CGUIWindowMusicFiles(void);
	
	virtual bool OnMessage(CGUIMessage& message);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
};

#endif //GUILIB_GUIWINDOWOMUSIC_H

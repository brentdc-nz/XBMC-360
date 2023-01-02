#ifndef GUILIB_GUIWINDOWPICTURES_H
#define GUILIB_GUIWINDOWPICTURES_H

#include "guilib/GUIMediaWindow.h"

class CGUIWindowPictures : public CGUIMediaWindow//, public IBackgroundLoaderObserver // TODO
{
public:
	CGUIWindowPictures(void);
	virtual ~CGUIWindowPictures(void);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
};

#endif //GUILIB_GUIWINDOWPICTURES_H
#ifndef GUILIB_GUIWINDOWVIDEOFILES_H
#define GUILIB_GUIWINDOWVIDEOFILES_H

#include "guilib\GUIWindowVideoBase.h"

namespace VIDEO
{
  class CVideoInfoScanner;
}

class CGUIWindowVideoFiles : public CGUIWindowVideoBase
{
public:
	CGUIWindowVideoFiles(void);
	virtual ~CGUIWindowVideoFiles(void);
	
	virtual bool OnMessage(CGUIMessage& message);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
};

#endif //GUILIB_GUIWINDOWVIDEOFILES_H

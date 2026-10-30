#ifndef GUILIB_GUIWINDOWOSD_H
#define GUILIB_GUIWINDOWOSD_H

#include "guilib\GUIDialog.h"

class CGUIWindowOSD : public CGUIDialog
{
public:

	CGUIWindowOSD(void);
	virtual ~CGUIWindowOSD(void);

	virtual void FrameMove();
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction &action);
protected:
	virtual void OnWindowLoaded();
};

#endif //GUILIB_GUIWINDOWOSD_H

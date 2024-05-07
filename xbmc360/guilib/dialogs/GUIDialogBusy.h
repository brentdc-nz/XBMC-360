#ifndef GUILIB_CGUIDIALOGBUSY_H
#define GUILIB_CGUIDIALOGBUSY_H

#include "guilib\GUIDialog.h"

class CGUIDialogBusy: public CGUIDialog
{
public:
	CGUIDialogBusy(void);
	virtual ~CGUIDialogBusy(void);

	virtual bool OnMessage(CGUIMessage& message);
	virtual void OnWindowLoaded();
	virtual void Render();
};

#endif //GUILIB_CGUIDIALOGBUSY_H
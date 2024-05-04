#ifndef GUILIB_CGUIDIALOGMUTEBUG_H
#define GUILIB_CGUIDIALOGMUTEBUG_H

#include "guilib\GUIDialog.h"

class CGUIDialogMuteBug : public CGUIDialog
{
public:
	CGUIDialogMuteBug(void);
	virtual ~CGUIDialogMuteBug(void);
	virtual bool OnMessage(CGUIMessage& message);
protected:
};

#endif //GUILIB_CGUIDIALOGMUTEBUG_H
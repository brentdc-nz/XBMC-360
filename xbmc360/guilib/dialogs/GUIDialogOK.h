#ifndef CGUIDIALOGOK_H
#define CGUIDIALOGOK_H

#include "guilib\GUIDialogBoxBase.h"

class CGUIDialogOK :
	public CGUIDialogBoxBase
{
public:
	CGUIDialogOK(void);
	virtual ~CGUIDialogOK(void);

	virtual bool OnMessage(CGUIMessage& message);
	static void ShowAndGetInput(int heading, int line0, int line1, int line2);
};

#endif //CGUIDIALOGOK_H
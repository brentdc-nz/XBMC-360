#ifndef GUILIB_CGUIDIALOGBUTTONMENU_H
#define GUILIB_CGUIDIALOGBUTTONMENU_H

#include "..\GUIDialog.h"

class CGUIDialogButtonMenu : public CGUIDialog
{
public:
	CGUIDialogButtonMenu(void);
	virtual ~CGUIDialogButtonMenu(void);

	virtual bool OnMessage(CGUIMessage& message);
};

#endif //GUILIB_CGUIDIALOGBUTTONMENU_H
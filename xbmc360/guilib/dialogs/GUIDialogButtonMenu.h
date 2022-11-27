#ifndef GUILIB_CGUIDIALOGBUTTONMENU_H
#define GUILIB_CGUIDIALOGBUTTONMENU_H

#include "guilib\GUIDialog.h"

class CGUIDialogButtonMenu :
	public CGUIDialog
{
public:
	CGUIDialogButtonMenu(int id = WINDOW_DIALOG_BUTTON_MENU, const CStdString &xmlFile = "DialogButtonMenu.xml");
	virtual ~CGUIDialogButtonMenu(void);

	virtual bool OnMessage(CGUIMessage &message);
	virtual void FrameMove();
};

#endif //GUILIB_CGUIDIALOGBUTTONMENU_H
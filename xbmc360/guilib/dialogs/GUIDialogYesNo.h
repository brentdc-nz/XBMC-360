#ifndef GUILIB_CGUIDIALOGYESNO_H
#define GUILIB_CGUIDIALOGYESNO_H

#include "..\GUIDialogBoxBase.h"

class CGUIDialogYesNo : public CGUIDialogBoxBase
{
public:
	CGUIDialogYesNo();
	virtual ~CGUIDialogYesNo();

	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction& action);

	static bool ShowAndGetInput(int heading, int line0, int line1, int line2, int iNoLabel=-1, int iYesLabel=-1);
	static bool ShowAndGetInput(int heading, int line0, int line1, int line2, int iNoLabel, int iYesLabel, bool& bCanceled);

protected:
	bool m_bCanceled;
};

#endif //GUILIB_CGUIDIALOGYESNO_H
#ifndef GUILIB_CGUIDIALOGBOXBASE_H
#define GUILIB_CGUIDIALOGBOXBASE_H

#include "guilib\GUIDialog.h"

class CGUIDialogBoxBase : public CGUIDialog
{
public:
	CGUIDialogBoxBase(int id, const CStdString &xmlFile);
	virtual ~CGUIDialogBoxBase(void);

	virtual bool OnMessage(CGUIMessage& message);
	bool IsConfirmed() const;
	void SetLine(int iLine, const string& strLine);
	void SetLine(int iLine, int iString);
	void SetHeading(const string& strLine);
	void SetHeading(int iString);
	void SetChoice(int iButton, int iString);
	void SetChoice(int iButton, const string& strString);

protected:
	virtual void OnInitWindow();
	bool m_bConfirmed;
};

#endif //GUILIB_CGUIDIALOGBOXBASE_H
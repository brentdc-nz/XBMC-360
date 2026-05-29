#ifndef GUILIB_GUIWINDOWSCRIPTSINFO_H
#define GUILIB_GUIWINDOWSCRIPTSINFO_H

#include "guilib\GUIDialog.h"

class CGUIWindowScriptsInfo : public CGUIDialog
{
public:
	CGUIWindowScriptsInfo(void);
	virtual ~CGUIWindowScriptsInfo(void);
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction &action);
	void AddText(const CStdString& strLabel);

protected:
	CStdString strInfo;
};

#endif //GUILIB_GUIWINDOWSCRIPTSINFO_H

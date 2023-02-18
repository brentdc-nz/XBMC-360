#ifndef GUILIB_GUIWINDOWSETTINGSCATEGORY_H
#define GUILIB_GUIWINDOWSETTINGSCATEGORY_H

#include "guilib\GUIWindow.h"
//#include "settings\SettingsControls.h"
#include "Settings.h"
#include "utils\Stopwatch.h"

class CGUIWindowSettingsCategory : public CGUIWindow
{
public:
	CGUIWindowSettingsCategory(void);
	virtual ~CGUIWindowSettingsCategory(void);

	virtual bool OnBack(int actionID);
	virtual bool OnMessage(CGUIMessage &message);

protected:
	virtual void OnInitWindow();
	virtual void SetupControls();
	virtual void FreeControls();
};

#endif //GUILIB_GUIWINDOWSETTINGSCATEGORY_H

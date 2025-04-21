#ifndef GUILIB_GUIWINDOWWEATHER_H
#define GUILIB_GUIWINDOWWEATHER_H

#include "guilib\GUIMediaWindow.h"
#include "utils\Stopwatch.h"

class CGUIWindowWeather : public CGUIWindow
{
public:
	CGUIWindowWeather(void);
	virtual ~CGUIWindowWeather(void);
	virtual bool OnMessage(CGUIMessage& message);
	virtual void FrameMove();

protected:
	virtual void OnInitWindow();

	void UpdateButtons();
	void UpdateLocations();
	void SetProperties();

	void Refresh();

	unsigned int m_iCurWeather;
};

#endif //GUILIB_GUIWINDOWWEATHER_H
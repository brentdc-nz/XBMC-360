#ifndef H_GUILIB_CGUIWINDOWSYSTEMINFO
#define H_GUILIB_CGUIWINDOWSYSTEMINFO

#include "guilib\GUIWindow.h"

#define CONTROL_BT_HDD      92
#define CONTROL_BT_DVD      93
#define CONTROL_BT_STORAGE  94
#define CONTROL_BT_DEFAULT  95
#define CONTROL_BT_NETWORK  96
#define CONTROL_BT_VIDEO    97
#define CONTROL_BT_HARDWARE 98

class CGUIWindowSystemInfo : public CGUIWindow
{
public:
	CGUIWindowSystemInfo(void);
	virtual ~CGUIWindowSystemInfo(void);
	virtual bool OnMessage(CGUIMessage& message);
	virtual void FrameMove();

private:
	unsigned int iControl;
	void SetLabelDummy();
	void SetControlLabel(int id, const char *format, int label, int info);
};

#endif //H_GUILIB_CGUIWINDOWSYSTEMINFO
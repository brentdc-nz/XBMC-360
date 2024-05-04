#ifndef GUILIB_CGUIDIALOGVOLUMEBAR_H
#define GUILIB_CGUIDIALOGVOLUMEBAR_H

#include "guilib\GUIDialog.h"

class CGUIDialogVolumeBar : public CGUIDialog
{
public:
	CGUIDialogVolumeBar(void);
	virtual ~CGUIDialogVolumeBar(void);

	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction &action);
	virtual void Render();
	void ResetTimer();

protected:
	unsigned int m_timer;
};

#endif //GUILIB_CGUIDIALOGVOLUMEBAR_H
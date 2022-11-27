#ifndef GUILIB_CGUIDIALOGSEEKBAR_H
#define GUILIB_CGUIDIALOGSEEKBAR_H

#include "guilib\GUIDialog.h"

class CGUIDialogSeekBar : public CGUIDialog
{
public:
	CGUIDialogSeekBar(void);
	virtual ~CGUIDialogSeekBar(void);

	virtual void Render();
	void ResetTimer();

protected:
	unsigned int m_timer;
};

#endif //GUILIB_CGUIDIALOGSEEKBAR_H
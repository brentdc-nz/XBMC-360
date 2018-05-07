#ifndef GUILIB_CGUIDIALOGSEEKBAR_H
#define GUILIB_CGUIDIALOGSEEKBAR_H

#include "..\GUIDialog.h"

class CGUIDialogSeekBar : public CGUIDialog
{
public:
	CGUIDialogSeekBar(void);
	virtual ~CGUIDialogSeekBar(void);

	virtual void Render();
	void ResetTimer();

protected:
	DWORD m_dwTimer;
};

#endif //GUILIB_CGUIDIALOGSEEKBAR_H
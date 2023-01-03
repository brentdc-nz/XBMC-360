#ifndef GUILIB_GUIWINDOWPROGRAMS_H
#define GUILIB_GUIWINDOWPROGRAMS_H

#include "guilib\GUIMediaWindow.h"
#include "guilib\dialogs\GUIDialogProgress.h"

class CGUIWindowPrograms :
      public CGUIMediaWindow//, public IBackgroundLoaderObserver
{
public:
	CGUIWindowPrograms(void);
	virtual ~CGUIWindowPrograms(void);

	virtual bool OnMessage(CGUIMessage& message);

protected:
	virtual bool OnPlayMedia(int iItem);
	virtual bool GetDirectory(const CStdString &strDirectory, CFileItemList &items);
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);

	CGUIDialogProgress* m_dlgProgress;
};

#endif //GUILIB_GUIWINDOWPROGRAMS_H
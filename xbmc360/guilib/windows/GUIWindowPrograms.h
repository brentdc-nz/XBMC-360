#ifndef GUILIB_GUIWINDOWPROGRAMS_H
#define GUILIB_GUIWINDOWPROGRAMS_H

#include "guilib\GUIMediaWindow.h"

class CGUIWindowPrograms :
      public CGUIMediaWindow//, public IBackgroundLoaderObserver
{
public:
	CGUIWindowPrograms(void);
	virtual ~CGUIWindowPrograms(void);

	virtual bool OnMessage(CGUIMessage& message);

protected:
	virtual bool GetDirectory(const CStdString &strDirectory, CFileItemList &items);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
	virtual bool OnPlayMedia(int iItem);
};

#endif //GUILIB_GUIWINDOWPROGRAMS_H
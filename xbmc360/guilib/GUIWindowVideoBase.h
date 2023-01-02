#include "GUIMediaWindow.h"

class CGUIWindowVideoBase : public CGUIMediaWindow//, public IBackgroundLoaderObserver, public IStreamDetailsObserver
{
public:
	CGUIWindowVideoBase(int id, const CStdString &xmlFile);
	virtual ~CGUIWindowVideoBase(void);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
};
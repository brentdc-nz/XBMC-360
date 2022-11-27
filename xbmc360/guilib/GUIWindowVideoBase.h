#include "GUIMediaWindow.h"

class CGUIWindowVideoBase : public CGUIMediaWindow//, public IBackgroundLoaderObserver, public IStreamDetailsObserver
{
public:
	CGUIWindowVideoBase(int id, const CStdString &xmlFile);
	virtual ~CGUIWindowVideoBase(void);

};
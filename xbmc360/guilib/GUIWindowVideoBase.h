#include "GUIMediaWindow.h"
#include "ThumbLoader.h"
#include "video\Bookmark.h"

class CGUIWindowVideoBase : public CGUIMediaWindow//, public IBackgroundLoaderObserver, public IStreamDetailsObserver
{
public:
	CGUIWindowVideoBase(int id, const CStdString &xmlFile);
	virtual ~CGUIWindowVideoBase(void);

	virtual bool OnClick(int iItem);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
	virtual bool Update(const CStdString &strDirectory);

	bool OnResumeShowMenu(CFileItem &item);
	void OnResumeItem(int iItem);
	void OnRestartItem(int iItem);

	CVideoThumbLoader m_thumbLoader;
};
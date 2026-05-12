#include "GUIMediaWindow.h"
#include "ThumbLoader.h"
#include "video\VideoDatabase.h"
#include "video\Bookmark.h"

class CGUIWindowVideoBase : public CGUIMediaWindow//, public IBackgroundLoaderObserver, public IStreamDetailsObserver
{
public:
	CGUIWindowVideoBase(int id, const CStdString &xmlFile);
	virtual ~CGUIWindowVideoBase(void);

	virtual bool OnClick(int iItem);

	int GetResumeItemOffset(const CFileItem *item);

	static bool OnResumeShowMenu(CFileItem &item);

protected:
	virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
	virtual bool Update(const CStdString &strDirectory);

	virtual void OnQueueItem(int iItem);
	void AddItemToPlayList(const CFileItemPtr &pItem, CFileItemList &queuedItems);
	void PlayItem(int iItem);
	void OnResumeItem(int iItem);
	void OnRestartItem(int iItem);

	CVideoDatabase m_database;
	CVideoThumbLoader m_thumbLoader;
};
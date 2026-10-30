#ifndef GUILIB_GUIDIALOGVIDEOBOOKMARKS_H
#define GUILIB_GUIDIALOGVIDEOBOOKMARKS_H

#include "guilib\GUIDialog.h"
#include "guilib\GUIViewControl.h"
#include "video\VideoDatabase.h"
#include "video\VideoInfoTag.h"

class CFileItemList;

class CGUIDialogVideoBookmarks : public CGUIDialog
{
public:
	CGUIDialogVideoBookmarks(void);
	virtual ~CGUIDialogVideoBookmarks(void);
	virtual bool OnMessage(CGUIMessage& message);
	virtual void OnWindowLoaded();
	virtual void OnWindowUnload();

protected:
	void GotoBookmark(int iItem);
	void ClearBookmarks();
	void AddBookmark(CVideoInfoTag* tag=NULL);
	void Clear();
	void Update();
	void AddEpisodeBookmark();

	CGUIControl *GetFirstFocusableControl(int id);

	CFileItemList* m_vecItems;
	CGUIViewControl m_viewControl;
	VECBOOKMARKS m_bookmarks;
};

#endif //GUILIB_GUIDIALOGVIDEOBOOKMARKS_H

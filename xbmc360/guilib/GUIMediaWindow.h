#ifndef GUILIB_GUIMEDIAWINDOW_H
#define GUILIB_GUIMEDIAWINDOW_H

#include "GUIWindow.h"
#include "GUIViewControl.h"
#include "..\FileItem.h"
#include "viewstates\GUIViewState.h"
#include "..\filesystem\VirtualDirectory.h"
#include "..\filesystem\DirectoryHistory.h"

// Base class for all media windows
class CGUIMediaWindow : public CGUIWindow
{
public:
	CGUIMediaWindow(int id, const CStdString &xmlFile);
	virtual ~CGUIMediaWindow(void);

	virtual void OnWindowLoaded();
	virtual void OnWindowUnload();
	virtual void OnInitWindow();

	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnBack(int actionID);

	bool OnClick(int iItem);
	virtual bool OnPlayMedia(int iItem);
	bool OnPopupMenu(int iItem);
	CStdString GetWindowType();

protected:
	virtual void OnSort();
	virtual bool GetDirectory(const CStdString &strDirectory, CFileItemList &items);
	void GetDirectoryHistoryString(const CFileItem* pItem, CStdString& strHistoryString);
	void SetHistoryForPath(const CStdString& strDirectory);
	virtual bool Update(const CStdString &strDirectory);
	virtual void ClearFileItems(); // Cleanup of items
	virtual void GoParentFolder();
	void UpdateButtons();
	void SetupShares();

	CGUIViewControl m_viewControl;
	DIRECTORY::CVirtualDirectory m_rootDir;

	// Current path and history
	CFileItemList m_vecItems;
	CDirectoryHistory m_history;
	auto_ptr<CGUIViewState> m_guiState;
};

#endif //GUILIB_GUIMEDIAWINDOW
#ifndef GUILIB_CGUIDIALOGFILEBROWSER_H
#define GUILIB_CGUIDIALOGFILEBROWSER_H

#include "..\GUIDialog.h"
#include "..\..\Settings.h"
#include "..\GUIViewControl.h"
#include "..\..\MediaManager.h"
#include "..\..\filesystem\VirtualDirectory.h"
#include "..\..\filesystem\DirectoryHistory.h"

class CGUIDialogFileBrowser : public CGUIDialog
{
public:
	CGUIDialogFileBrowser(void);
	virtual ~CGUIDialogFileBrowser(void);
	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);
	virtual void OnWindowLoaded();
	virtual void OnWindowUnload();
	bool IsConfirmed() { return m_bConfirmed; };
	static bool ShowAndGetShare(CStdString &path, bool allowNetworkShares/*, VECSHARES* additionalShare = NULL*/);
	void SetShares(VECSOURCES &shares);
	void OnAddNetworkLocation();

protected:
	void OnClick(int iItem);
	void ClearFileItems();
	void Update(const CStdString &strDirectory);

	CDirectoryHistory m_history;
	bool m_singleList; // If true, we have no shares or anything
	int m_browsingForFolders; // 0 - no, 1 - yes, 2 - yes, only writable
	bool m_bConfirmed;
	CFileItem m_Directory;
	CStdString m_strParentPath;
	CStdString m_selectedPath;
	VECSOURCES m_shares;
	bool m_useFileDirectories;
	DIRECTORY::CVirtualDirectory m_rootDir;
	CFileItemList m_vecItems;
	CGUIViewControl m_viewControl;
};

#endif //GUILIB_CGUIDIALOGFILEBROWSER_H
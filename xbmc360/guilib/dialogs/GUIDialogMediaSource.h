#ifndef CDIALOGMEDIASOURCES_H
#define CDIALOGMEDIASOURCES_H

#include "..\GUIDialog.h"
#include "..\..\FileItem.h"
#include "..\..\MediaManager.h"

class CGUIDialogMediaSource : public CGUIDialog
{
public:
	CGUIDialogMediaSource(void);
	virtual ~CGUIDialogMediaSource(void);

	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);
	static bool ShowAndAddMediaSource(const CStdString &type);
	void SetTypeOfMedia(const CStdString &type, bool editNotAdd = false);
	void SetShare(const CMediaSource &share);
	bool IsConfirmed() const { return m_confirmed; };

protected:
	int GetSelectedItem();
	void OnPathBrowse(int item);
	void UpdateButtons();
	void HighlightItem(int item);
	void OnPathAdd();
	void OnPathRemove(int item);
	void OnOK();
	void OnCancel();
	vector<CStdString> GetPaths();

	bool m_confirmed;
	CStdString m_name;
	CStdString m_type;
	CFileItemList m_paths;
};

#endif //CDIALOGMEDIASOURCES_H
#ifndef CDIALOGMEDIASOURCES_H
#define CDIALOGMEDIASOURCES_H

#include "guilib\GUIDialog.h"
//#include "video/VideoInfoScanner.h" // TODO

class CFileItemList;
class CMediaSource;

class CGUIDialogMediaSource : public CGUIDialog
{
public:
	CGUIDialogMediaSource(void);
	virtual ~CGUIDialogMediaSource(void);
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnBack(int actionID);
	virtual void OnWindowLoaded();
	static bool ShowAndAddMediaSource(const CStdString &type);
	static bool ShowAndEditMediaSource(const CStdString &type, const CMediaSource &share);
	static bool ShowAndEditMediaSource(const CStdString &type, const CStdString &share);

	bool IsConfirmed() const { return m_confirmed; };

	void SetShare(const CMediaSource &share);
	void SetTypeOfMedia(const CStdString &type, bool editNotAdd = false);

protected:
	void OnPathBrowse(int item);
	void OnPath(int item);
	void OnPathAdd();
	void OnPathRemove(int item);
	void OnOK();
	void OnCancel();
	void UpdateButtons();
	int GetSelectedItem();
	void HighlightItem(int item);

	std::vector<CStdString> GetPaths();

	CStdString m_type;
	CStdString m_name;
	CFileItemList* m_paths;
	bool m_confirmed;
//	SScraperInfo m_info; // TODO
	bool m_bRunScan;
	bool m_bNameChanged;

//	VIDEO::SScanSettings m_settings; // TODO
};

#endif //CDIALOGMEDIASOURCES_H
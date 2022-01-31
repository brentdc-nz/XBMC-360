#include "..\GUIDialog.h"
#include "..\..\FileItem.h"

#ifndef GUILIB_CGUIDIALOGCONTEXTMENU_H
#define GUILIB_CGUIDIALOGCONTEXTMENU_H

enum CONTEXT_BUTTON { CONTEXT_BUTTON_CANCELLED = 0, // WIP: More to be added
                      CONTEXT_BUTTON_EDIT_SOURCE,
					  CONTEXT_BUTTON_ADD_SOURCE,
                      CONTEXT_BUTTON_REMOVE_SOURCE
                    };

class CContextButtons : public std::vector<std::pair<CONTEXT_BUTTON, CStdString>>
{
public:
	void Add(CONTEXT_BUTTON, const CStdString &label);
	void Add(CONTEXT_BUTTON, int label);
};

class CGUIDialogContextMenu : public CGUIDialog
{
public:
	CGUIDialogContextMenu(void);
	virtual ~CGUIDialogContextMenu(void);

	virtual void OnInitWindow();
	virtual void OnWindowUnload();
	virtual bool OnMessage(CGUIMessage& message);
	virtual void DoModal(int iWindowID = WINDOW_INVALID);
	static bool SourcesMenu(const CStdString &strType, const CFileItem *item, float posX, float posY);
	static void GetContextButtons(const CStdString &type, const CFileItem *item, CContextButtons &buttons);
	static bool OnContextButton(const CStdString &type, const CFileItem *item, CONTEXT_BUTTON button);
	static CMediaSource *GetShare(const CStdString &type, const CFileItem *item);

protected:
	int AddButton(const CStdString &strLabel);
	void ClearButtons();
	int GetButton();

private:
	int m_iNumButtons;
	int m_iClickedButton;
};

#endif //GUILIB_CGUIDIALOGCONTEXTMENU_H
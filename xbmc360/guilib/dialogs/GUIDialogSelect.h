#pragma once

#include "GUIDialogBoxBase.h"
#include "guilib\GUIListItem.h"
#include "guilib\GUIMessage.h"

class CFileItem;
class CFileItemList;

class CGUIDialogSelect :
	public CGUIDialogBoxBase
{
public:
	CGUIDialogSelect(void);
	virtual ~CGUIDialogSelect(void);
	virtual bool OnMessage(CGUIMessage& message);

	void Reset();
	void Add(const CStdString& strLabel);
	void Add(const CFileItem* pItem);
	void Add(const CFileItemList& items);
	void SetItems(CFileItemList* items);
	int GetSelectedLabel() const;
	const CStdString& GetSelectedLabelText();
	const CFileItem& GetSelectedItem();
	void EnableButton(bool bOnOff);
	void SetButtonLabel(int iString);
	bool IsButtonPressed();
	void Sort(bool bSortOrder = true);
	void SetSelected(int iSelected);

protected:
	bool m_bButtonEnabled;
	bool m_bButtonPressed;
	int m_iSelected;

	CFileItem* m_selectedItem;
	CFileItemList* m_vecListInternal;
	CFileItemList* m_vecList;
};
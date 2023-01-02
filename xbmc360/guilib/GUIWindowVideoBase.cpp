#include "GUIWindowVideoBase.h"

CGUIWindowVideoBase::CGUIWindowVideoBase(int id, const CStdString &xmlFile)
	: CGUIMediaWindow(id, xmlFile)
{
}

CGUIWindowVideoBase::~CGUIWindowVideoBase()
{
}

void CGUIWindowVideoBase::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	CFileItemPtr item;
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	// TODO

	CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
}

bool CGUIWindowVideoBase::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	CFileItemPtr item;
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	// TODO

	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}
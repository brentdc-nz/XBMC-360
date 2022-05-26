#include "GUIDialogContextMenu.h"
#include "..\GUIWindowManager.h"
#include "..\LocalizeStrings.h"
#include "..\..\Settings.h"
#include "..\GUIButtonControl.h"
#include "GUIDialogMediaSource.h"
#include "GUIDialogYesNo.h"

#define BACKGROUND_BOTTOM      998
#define BACKGROUND_IMAGE       999
#define BUTTON_TEMPLATE        1000
#define SPACE_BETWEEN_BUTTONS  4

void CContextButtons::Add(CONTEXT_BUTTON button, const CStdString &label)
{
	push_back(pair<CONTEXT_BUTTON, CStdString>(button, label));
}

void CContextButtons::Add(CONTEXT_BUTTON button, int label)
{
	push_back(pair<CONTEXT_BUTTON, CStdString>(button, g_localizeStrings.Get(label)));
}

CGUIDialogContextMenu::CGUIDialogContextMenu(void)
	: CGUIDialog(WINDOW_DIALOG_CONTEXT_MENU, "DialogContextMenu.xml")
{
	m_iClickedButton = -1;
	m_iNumButtons = 0;
}

CGUIDialogContextMenu::~CGUIDialogContextMenu(void)
{
}

void CGUIDialogContextMenu::OnInitWindow()
{
	// Disable the template button control
	CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE);

	if(pControl)
		pControl->SetVisible(false);
	
	m_iClickedButton = -1;
	// Set initial control focus
	m_lastControlID = BUTTON_TEMPLATE + 1;

	CGUIDialog::OnInitWindow();
}

void CGUIDialogContextMenu::OnWindowUnload()
{
	ClearButtons();
}

bool CGUIDialogContextMenu::OnMessage(CGUIMessage &message)
{
	if(message.GetMessage() == GUI_MSG_CLICKED)
	{
		// Someone has been clicked - deinit...
		m_iClickedButton = message.GetSenderId() - BUTTON_TEMPLATE;
		Close();
		return true;
	}
	return CGUIDialog::OnMessage(message);
}

void CGUIDialogContextMenu::DoModal(int iWindowID /*= WINDOW_INVALID*/)
{
	// Update the navigation of the first and last buttons
	CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE + 1);
	if(pControl)
		pControl->SetNavigation(BUTTON_TEMPLATE + m_iNumButtons, pControl->GetControlIdDown(), pControl->GetControlIdLeft(), pControl->GetControlIdRight());
	
	pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE + m_iNumButtons);
	if(pControl)
		pControl->SetNavigation(pControl->GetControlIdUp(), BUTTON_TEMPLATE + 1, pControl->GetControlIdLeft(), pControl->GetControlIdRight());
/*
	// Update our default control
	if (m_defaultControl <= BUTTON_TEMPLATE || m_defaultControl > (BUTTON_TEMPLATE + m_iNumButtons))
		m_defaultControl = BUTTON_TEMPLATE + 1;
	
	// check the default control has focus...
	while (m_defaultControl <= (BUTTON_TEMPLATE + m_iNumButtons) && !(GetControl(m_defaultControl)->CanFocus()))
		m_defaultControl++;
*/
	CGUIDialog::DoModal();
}

bool CGUIDialogContextMenu::SourcesMenu(const CStdString &strType, const CFileItem *item, float posX, float posY)
{
	if(!item)
		return false;

	// Popup the context menu
	CGUIDialogContextMenu *pMenu = (CGUIDialogContextMenu *)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
	if(pMenu)
	{
		// Load our menu
		pMenu->Initialize();

		// Grab our context menu
		CContextButtons buttons;
		GetContextButtons(strType, item, buttons);

		// Add the buttons and execute it
		for(CContextButtons::iterator it = buttons.begin(); it != buttons.end(); it++)
			pMenu->AddButton((*it).second);
		
		// Position it correctly
//		pMenu->OffsetPosition(posX, posY);
		pMenu->DoModal();

		// Translate our button press
		CONTEXT_BUTTON btn = CONTEXT_BUTTON_CANCELLED;

		if(pMenu->GetButton() > 0 && pMenu->GetButton() <= (int)buttons.size())
			btn = buttons[pMenu->GetButton() - 1].first;
		
		if(btn != CONTEXT_BUTTON_CANCELLED)
			return OnContextButton(strType, item, btn);
	}

	return false;
}

void CGUIDialogContextMenu::GetContextButtons(const CStdString &type, const CFileItem *item, CContextButtons &buttons)
{
	// Add buttons to the ContextMenu that should be visible for both sources and autosourced items
	
	// WIP..
	
	// Next, Add buttons to the ContextMenu that should ONLY be visible for sources and not autosourced items
	CMediaSource *share = GetShare(type, item);

	if(share)
	{
		if(/*!share->m_ignore*/1)
			buttons.Add(CONTEXT_BUTTON_EDIT_SOURCE, 1027); // Edit Source

		if(/*!share->m_ignore*/1)
			buttons.Add(CONTEXT_BUTTON_REMOVE_SOURCE, 522); // Remove Source
	}
	
//	if(!GetDefaultShareNameByType(type).IsEmpty())
//		buttons.Add(CONTEXT_BUTTON_CLEAR_DEFAULT, 13403); // Clear Default

	buttons.Add(CONTEXT_BUTTON_ADD_SOURCE, 1026); // Add Source
}

bool CGUIDialogContextMenu::OnContextButton(const CStdString &type, const CFileItem *item, CONTEXT_BUTTON button)
{
	// Add Source doesn't require a valid share
	if(button == CONTEXT_BUTTON_ADD_SOURCE)
		return CGUIDialogMediaSource::ShowAndAddMediaSource(type);

	// The rest of the operations require a valid share
	CMediaSource *share = GetShare(type, item);
	if(!share) return false;

	switch (button)
	{
//		case CONTEXT_BUTTON_EDIT_SOURCE:
//			return CGUIDialogMediaSource::ShowAndEditMediaSource(type, *share);
    
		case CONTEXT_BUTTON_REMOVE_SOURCE:
			// Prompt user if they want to really delete the source
			if(CGUIDialogYesNo::ShowAndGetInput(751, 0, 750, 0))
			{
				// Check default before we delete, as deletion will kill the share object
/*				CStdString defaultSource(GetDefaultShareNameByType(type));
				if(!defaultSource.IsEmpty())
				{
					if(share->strName.Equals(defaultSource))
						ClearDefault(type);
				}
*/
				// Delete this share
				g_settings.DeleteSource(type, share->strName, share->strPath);
				return true;
			}
		break;

	default:
		break;
	}

	return false;
}

CMediaSource *CGUIDialogContextMenu::GetShare(const CStdString &type, const CFileItem *item)
{
	VECSOURCES *shares = g_settings.GetSourcesFromType(type);

	if(!shares) return NULL;
	
	for(unsigned int i = 0; i < shares->size(); i++)
	{
		CMediaSource &testShare = shares->at(i);
/*		if(CUtil::IsDVD(testShare.strPath))
		{
			if(!item->IsDVD())
				continue;
		}
		else
*/		{
			if(!testShare.strPath.Equals(item->m_strPath))
				continue;
		}

		// Paths match, what about share name - only match the leftmost
		// characters as the label may contain other info (status for instance)
		if(item->GetLabel().Left(testShare.strName.size()).Equals(testShare.strName))
			return &testShare;
	}	
	return NULL;
}

int CGUIDialogContextMenu::AddButton(const CStdString &strLabel)
{
	// Add a button to our control
	CGUIButtonControl *pButtonTemplate = NULL;//(CGUIButtonControl*)GetFirstFocusableControl(BUTTON_TEMPLATE);
	if(!pButtonTemplate) pButtonTemplate = (CGUIButtonControl*)GetControl(BUTTON_TEMPLATE);

	if(!pButtonTemplate) return 0;

	CGUIButtonControl *pButton = new CGUIButtonControl(*pButtonTemplate);
	if(!pButton) return 0;

	// Set the button's ID and position
	m_iNumButtons++;

	int id = BUTTON_TEMPLATE + m_iNumButtons;
	pButton->SetID(id);

	pButton->SetPosition(pButtonTemplate->GetXPosition(), pButtonTemplate->GetYPosition() + (m_iNumButtons - 1)*(pButtonTemplate->GetHeight() + SPACE_BETWEEN_BUTTONS));
	pButton->SetVisible(true);
	pButton->SetNavigation(id - 1, id + 1, id, id);
	pButton->SetLabel(strLabel);
	AddControl(pButton);

	// And update the size of our menu
	CGUIControl *pControl = (CGUIControl *)GetControl(BACKGROUND_IMAGE);
	if(pControl)
	{
		pControl->SetHeight(m_iNumButtons*(pButtonTemplate->GetHeight() + SPACE_BETWEEN_BUTTONS));
		CGUIControl *pControl2 = (CGUIControl *)GetControl(BACKGROUND_BOTTOM);
		
		if(pControl2)
			pControl2->SetPosition(pControl2->GetXPosition(), pControl->GetYPosition() + pControl->GetHeight());
	}

 	return m_iNumButtons;
}

void CGUIDialogContextMenu::ClearButtons()
{ 
	// Destroy our buttons (if we have them from a previous viewing)
	for (int i = 1; i <= m_iNumButtons; i++)
	{
		// Get the button to remove...
		CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE + i);
		if (pControl)
		{
			// Remove the control from our list
			RemoveControl(pControl->GetID());

			// Kill the button
			pControl->FreeResources();
			delete pControl;
		}
	}
	m_iNumButtons = 0;
}

int CGUIDialogContextMenu::GetButton()
{
	return m_iClickedButton;
}
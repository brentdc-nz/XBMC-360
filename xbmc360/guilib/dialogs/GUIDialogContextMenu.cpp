#include "GUIDialogContextMenu.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\LocalizeStrings.h"
#include "Settings.h"
#include "guilib\GUIButtonControl.h"
#include "GUIDialogMediaSource.h"
#include "GUIDialogYesNo.h"

#define BACKGROUND_IMAGE       999
#define BACKGROUND_BOTTOM      998
#define BACKGROUND_TOP         997
#define BUTTON_TEMPLATE       1000
#define BUTTON_START          1001
#define BUTTON_END            (BUTTON_START + (int)m_buttons.size() - 1)
#define SPACE_BETWEEN_BUTTONS    2

void CContextButtons::Add(unsigned int button, const CStdString &label)
{
	push_back(pair<unsigned int, CStdString>(button, label));
}

void CContextButtons::Add(unsigned int button, int label)
{
	push_back(pair<unsigned int, CStdString>(button, g_localizeStrings.Get(label)));
}

CGUIDialogContextMenu::CGUIDialogContextMenu(void):CGUIDialog(WINDOW_DIALOG_CONTEXT_MENU, "DialogContextMenu.xml")
{
	m_clickedButton = -1;
}

CGUIDialogContextMenu::~CGUIDialogContextMenu(void)
{
}

bool CGUIDialogContextMenu::OnMessage(CGUIMessage &message)
{
	if (message.GetMessage() == GUI_MSG_CLICKED)
	{
		// Someone has been clicked - deinit...
		if (message.GetSenderId() >= BUTTON_START && message.GetSenderId() <= BUTTON_END)
			m_clickedButton = (int)m_buttons[message.GetSenderId() - BUTTON_START].first;
		
		Close();
		return true;
	}
	return CGUIDialog::OnMessage(message);
}

void CGUIDialogContextMenu::OnInitWindow()
{
	// Disable the template button control
	CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE);
	if (pControl)
	{
		pControl->SetVisible(false);
	}
	
	m_clickedButton = -1;

	// Set initial control focus
	m_lastControlID = BUTTON_START;
	CGUIDialog::OnInitWindow();
}

void CGUIDialogContextMenu::DoModal(int iWindowID /*= WINDOW_INVALID */, const CStdString &param)
{
	// Update the navigation of the first and last buttons
	CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_START);
	
	if (pControl)
		pControl->SetNavigation(BUTTON_END, pControl->GetControlIdDown(), pControl->GetControlIdLeft(), pControl->GetControlIdRight());
	
	pControl = (CGUIControl *)GetControl(BUTTON_END);
	if (pControl)
		pControl->SetNavigation(pControl->GetControlIdUp(), BUTTON_START, pControl->GetControlIdLeft(), pControl->GetControlIdRight());
	
	// Update our default control
	if (m_defaultControl < BUTTON_START || m_defaultControl > BUTTON_END)
		m_defaultControl = BUTTON_START;
	
	// Check the default control has focus...
	while (m_defaultControl <= BUTTON_END && !(GetControl(m_defaultControl)->CanFocus()))
		m_defaultControl++;
	
	CGUIDialog::DoModal();
}

void CGUIDialogContextMenu::OnWindowLoaded()
{
	CGUIDialog::OnWindowLoaded();
	SetInitialVisibility();
}

void CGUIDialogContextMenu::OnWindowUnload()
{
	ClearButtons();
}

void CGUIDialogContextMenu::SetPosition(float posX, float posY)
{
	if (posY + GetHeight() > g_settings.m_ResInfo[m_coordsRes].iHeight)
		posY = g_settings.m_ResInfo[m_coordsRes].iHeight - GetHeight();
	
	if (posY < 0) posY = 0;
		if (posX + GetWidth() > g_settings.m_ResInfo[m_coordsRes].iWidth)
			posX = g_settings.m_ResInfo[m_coordsRes].iWidth - GetWidth();
	
	if (posX < 0) posX = 0;
	
	// We currently hack the positioning of the buttons from y position 0, which
	// forces skinners to place the top image at a negative y value. Thus, we offset
	// the y coordinate by the height of the top image.
	const CGUIControl *top = GetControl(BACKGROUND_TOP);
	
	if (top)
		posY += top->GetHeight();
	
	CGUIDialog::SetPosition(posX, posY);
}

void CGUIDialogContextMenu::ClearButtons()
{
	// Destroy our buttons (if we have them from a previous viewing)
	for (unsigned int i = 0; i < m_buttons.size(); i++)
	{
		// Get the button to remove...
		CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_START + i);
		if (pControl)
		{
			// Remove the control from our list
			RemoveControl(pControl);

			// Kill the button
			pControl->FreeResources();
			delete pControl;
		}
	}
	m_buttons.clear();
}

int CGUIDialogContextMenu::AddButton(const CStdString &strLabel, int value /* = -1 */)
{
	// Add a button to our control
	CGUIButtonControl *pButtonTemplate = (CGUIButtonControl *)GetFirstFocusableControl(BUTTON_TEMPLATE);
	
	if (!pButtonTemplate) pButtonTemplate = (CGUIButtonControl *)GetControl(BUTTON_TEMPLATE);
	if (!pButtonTemplate) return 0;
	
	CGUIButtonControl *pButton = new CGUIButtonControl(*pButtonTemplate);
	if (!pButton) return 0;
	
	// Set the button's ID and position
	if (value < 0)
		value = m_buttons.size() + 1; // Default is to start at 1
	
	int id = BUTTON_START + m_buttons.size();
	m_buttons.Add(value, strLabel);
	pButton->SetID(id);
	pButton->SetPosition(pButtonTemplate->GetXPosition(), (m_buttons.size() - 1)*(pButtonTemplate->GetHeight() + SPACE_BETWEEN_BUTTONS));
	pButton->SetVisible(true);
	pButton->SetNavigation(id - 1, id + 1, id, id);
	pButton->SetLabel(strLabel);
	AddControl(pButton);
	
	// and update the size of our menu
	CGUIControl *pControl = (CGUIControl *)GetControl(BACKGROUND_IMAGE);
	if (pControl)
	{
		pControl->SetHeight(m_buttons.size() * (pButtonTemplate->GetHeight() + SPACE_BETWEEN_BUTTONS));
		CGUIControl *pControl2 = (CGUIControl *)GetControl(BACKGROUND_BOTTOM);

		if (pControl2)
			pControl2->SetPosition(pControl2->GetXPosition(), pControl->GetYPosition() + pControl->GetHeight());
	}
	return value;
}

int CGUIDialogContextMenu::GetButton()
{
	return m_clickedButton;
}

float CGUIDialogContextMenu::GetHeight()
{
	const CGUIControl *backMain = GetControl(BACKGROUND_IMAGE);
	
	if (backMain)
	{
		float height = backMain->GetHeight();
		const CGUIControl *backBottom = GetControl(BACKGROUND_BOTTOM);
		
		if (backBottom)
			height += backBottom->GetHeight();
		
		const CGUIControl *backTop = GetControl(BACKGROUND_TOP);
		
		if (backTop)
			height += backTop->GetHeight();
		
		return height;
	}
	else
		return CGUIDialog::GetHeight();
}

float CGUIDialogContextMenu::GetWidth()
{
	CGUIControl *pControl = (CGUIControl *)GetControl(BACKGROUND_IMAGE);

	if (pControl)
		return pControl->GetWidth();
	else
		return CGUIDialog::GetWidth();
}

void CGUIDialogContextMenu::OffsetPosition(float offsetX, float offsetY)
{
	float newX = m_posX + offsetX - GetWidth() * 0.5f;
	float newY = m_posY + offsetY - GetHeight() * 0.5f;
	SetPosition(newX, newY);
}

void CGUIDialogContextMenu::PositionAtCurrentFocus()
{
	CGUIWindow *window = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
	if (window)
	{
		const CGUIControl *focusedControl = window->GetFocusedControl();
		if (focusedControl)
		{
			CPoint pos = focusedControl->GetRenderPosition() + CPoint(focusedControl->GetWidth() * 0.5f, focusedControl->GetHeight() * 0.5f);
			OffsetPosition(pos.x,pos.y);
			return;
		}
	}
	// No control to center at, so just center the window
	CenterWindow();
}

bool CGUIDialogContextMenu::OnContextButton(const CStdString &type, const CFileItemPtr item, CONTEXT_BUTTON button)
{
	// Add Source doesn't require a valid share
	if (button == CONTEXT_BUTTON_ADD_SOURCE)
	{
/* // TODO: Locking
		if (g_settings.IsMasterUser())
		{
			if (!g_passwordManager.IsMasterLockUnlocked(true))
				return false;
		}
		else if (!g_settings.GetCurrentProfile().canWriteSources() && !g_passwordManager.IsProfileLockUnlocked())
			return false;
*/
		return CGUIDialogMediaSource::ShowAndAddMediaSource(type);
	}

#ifdef _HAS_DVDROM
	// Buttons that are available on both sources and autosourced items
	if (!item) return false;
	switch (button)
	{
		case CONTEXT_BUTTON_PLAY_DISC:
			return CAutorun::PlayDisc();

		case CONTEXT_BUTTON_EJECT_DISC:
			CIoSupport::ToggleTray();
			return true;

		default:
		break;
	}
#endif

	// The rest of the operations require a valid share
	CMediaSource *share = GetShare(type, item.get());
	if (!share) return false;
	
	switch (button)
	{
		case CONTEXT_BUTTON_EDIT_SOURCE:
/*			if (g_settings.IsMasterUser()) // TODO: Lockng
			{
				if (!g_passwordManager.IsMasterLockUnlocked(true))
					return false;
			}
			else if (!g_passwordManager.IsProfileLockUnlocked())
				return false;
*/
			return CGUIDialogMediaSource::ShowAndEditMediaSource(type, *share);
    
		case CONTEXT_BUTTON_REMOVE_SOURCE:
/*			if (g_settings.IsMasterUser()) // TODO: Lockng
			{
				if (!g_passwordManager.IsMasterLockUnlocked(true))
					return false;
			}
			else 
			{
				if (!g_settings.GetCurrentProfile().canWriteSources() && !g_passwordManager.IsMasterLockUnlocked(false))
					return false;
			
				if (g_settings.GetCurrentProfile().canWriteSources() && !g_passwordManager.IsProfileLockUnlocked())
					return false;
			}
*/		
			// Prompt user if they want to really delete the source
			if (CGUIDialogYesNo::ShowAndGetInput(751, 0, 750, 0))
			{
/*				// Check default before we delete, as deletion will kill the share object
				CStdString defaultSource(GetDefaultShareNameByType(type)); // TODO
				if (!defaultSource.IsEmpty())
				{
					if (share->strName.Equals(defaultSource))
						ClearDefault(type);
				}
*/
				// Delete this share
				g_settings.DeleteSource(type, share->strName, share->strPath);
				return true;
			}
			break;

		// TODO - More to be added!

		default:
			break;
	}
	return false;
}

void CGUIDialogContextMenu::GetContextButtons(const CStdString &type, const CFileItemPtr item, CContextButtons &buttons)
{
#ifdef _HAS_DVDROM
	// Add buttons to the ContextMenu that should be visible for both sources and autosourced items
	if (item && (item->IsDVD() || item->IsCDDA()))
	{
		// We need to check if there is a detected is inserted!
		if ( CDetectDVDMedia::IsDiscInDrive() )
			buttons.Add(CONTEXT_BUTTON_PLAY_DISC, 341); // Play CD/DVD!
		
		buttons.Add(CONTEXT_BUTTON_EJECT_DISC, 13391);  // Eject/Load CD/DVD!
	}
#endif

	// Next, Add buttons to the ContextMenu that should ONLY be visible for sources and not autosourced items
	CMediaSource *share = GetShare(type, item.get());

	if (/*g_settings.GetCurrentProfile().canWriteSources() || g_passwordManager.bMasterUser*/1) // TODO: Profiles
	{
		if (share)
		{
			if (!share->m_ignore)
				buttons.Add(CONTEXT_BUTTON_EDIT_SOURCE, 1027); // Edit Source
			
//			buttons.Add(CONTEXT_BUTTON_SET_DEFAULT, 13335); // Set as Default // TODO
			
			if (!share->m_ignore)
				buttons.Add(CONTEXT_BUTTON_REMOVE_SOURCE, 522); // Remove Source

//			buttons.Add(CONTEXT_BUTTON_SET_THUMB, 20019); // TODO
		}
		
//		if (!GetDefaultShareNameByType(type).IsEmpty())
//			buttons.Add(CONTEXT_BUTTON_CLEAR_DEFAULT, 13403); // Clear Default // TODO

		buttons.Add(CONTEXT_BUTTON_ADD_SOURCE, 1026); // Add Source
	}
	
/*	if (share && LOCK_MODE_EVERYONE != g_settings.GetMasterProfile().getLockMode()) // TODO: Locking
	{
		if (share->m_iHasLock == 0 && (g_settings.GetCurrentProfile().canWriteSources() || g_passwordManager.bMasterUser))
			buttons.Add(CONTEXT_BUTTON_ADD_LOCK, 12332);
		else if (share->m_iHasLock == 1)
			buttons.Add(CONTEXT_BUTTON_REMOVE_LOCK, 12335);
		else if (share->m_iHasLock == 2)
		{
			buttons.Add(CONTEXT_BUTTON_REMOVE_LOCK, 12335);

			bool maxRetryExceeded = false;

			if (g_guiSettings.GetInt("masterlock.maxretries") != 0)
				maxRetryExceeded = (share->m_iBadPwdCount >= g_guiSettings.GetInt("masterlock.maxretries"));
  
			if (maxRetryExceeded)
				buttons.Add(CONTEXT_BUTTON_RESET_LOCK, 12334);
			else
				buttons.Add(CONTEXT_BUTTON_CHANGE_LOCK, 12356);
		}
	}
	if (share && !g_passwordManager.bMasterUser && item->m_iHasLock == 1)
		buttons.Add(CONTEXT_BUTTON_REACTIVATE_LOCK, 12353);
*/
}

int CGUIDialogContextMenu::ShowAndGetChoice(const CContextButtons &choices)
{
	if (choices.size() == 0)
		return -1;

	CGUIDialogContextMenu *pMenu = (CGUIDialogContextMenu *)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
	if (pMenu)
	{
		pMenu->Initialize();

		for (unsigned int i = 0; i < choices.size(); i++)
			pMenu->AddButton(choices[i].second, choices[i].first);

		pMenu->PositionAtCurrentFocus();
		pMenu->DoModal();

		return pMenu->GetButton();
	}
	return -1;
}

CMediaSource *CGUIDialogContextMenu::GetShare(const CStdString &type, const CFileItem *item)
{
	VECSOURCES *shares = g_settings.GetSourcesFromType(type);
	if (!shares) return NULL;
	
	for (unsigned int i = 0; i < shares->size(); i++)
	{
		CMediaSource &testShare = shares->at(i);
#ifdef _HAS_DVDROM
		if (URIUtils::IsDVD(testShare.strPath))
		{
			if (!item->IsDVD())
				continue;
		}
		else
#endif
		{
			if (!testShare.strPath.Equals(item->GetPath()))
				continue;
		}
		
		// Paths match, what about share name - only match the leftmost
		// characters as the label may contain other info (status for instance)
		if (item->GetLabel().Left(testShare.strName.size()).Equals(testShare.strName))
			return &testShare;
	}
	return NULL;
}
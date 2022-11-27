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

bool CGUIDialogContextMenu::OnContextButton(const CStdString &type, const CFileItemPtr item, CONTEXT_BUTTON button)
{
	// Add Source doesn't require a valid share
	if (button == CONTEXT_BUTTON_ADD_SOURCE)
	{
		return CGUIDialogMediaSource::ShowAndAddMediaSource(type);
	}

	// TODO : More to be added!

	return false;
}
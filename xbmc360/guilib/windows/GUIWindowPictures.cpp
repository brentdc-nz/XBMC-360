#include "GUIWindowPictures.h"

CGUIWindowPictures::CGUIWindowPictures(void)
	: CGUIMediaWindow(WINDOW_PICTURES, "MyPictures.xml")
{
}

CGUIWindowPictures::~CGUIWindowPictures(void)
{
}

bool CGUIWindowPictures::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	CFileItemPtr item = (itemNumber >= 0 && itemNumber < m_vecItems->Size()) ? m_vecItems->Get(itemNumber) : CFileItemPtr();
	
	if (m_vecItems->IsVirtualDirectoryRoot() && item)
	{
		if (CGUIDialogContextMenu::OnContextButton("pictures", item, button))
		{
			Update("");
			return true;
		}
	}
/*	
	switch (button) // TODO
	{
		case CONTEXT_BUTTON_VIEW_SLIDESHOW:
			if (item && item->m_bIsFolder)
				OnSlideShow(item->GetPath());
			else
				ShowPicture(itemNumber, true);
		return true;
		case CONTEXT_BUTTON_RECURSIVE_SLIDESHOW:
			if (item)
				OnSlideShowRecursive(item->GetPath());
		return true;
		case CONTEXT_BUTTON_INFO:
			OnInfo(itemNumber);
		return true;
		case CONTEXT_BUTTON_REFRESH_THUMBS:
			OnRegenerateThumbs();
		return true;
		case CONTEXT_BUTTON_DELETE:
			OnDeleteItem(itemNumber);
		return true;
		case CONTEXT_BUTTON_RENAME:
			OnRenameItem(itemNumber);
		return true;
		case CONTEXT_BUTTON_SETTINGS:
			g_windowManager.ActivateWindow(WINDOW_SETTINGS_MYPICTURES);
		return true;
		case CONTEXT_BUTTON_GOTO_ROOT:
			Update("");
		return true;
		case CONTEXT_BUTTON_SWITCH_MEDIA:
			CGUIDialogContextMenu::SwitchMedia("pictures", m_vecItems->GetPath());
		return true;
		default:
			break;
	}
*/
	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

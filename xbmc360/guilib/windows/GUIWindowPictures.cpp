#include "GUIWindowPictures.h"

CGUIWindowPictures::CGUIWindowPictures(void)
	: CGUIMediaWindow(WINDOW_PICTURES, "MyPictures.xml")
{
}

CGUIWindowPictures::~CGUIWindowPictures(void)
{
}

void CGUIWindowPictures::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	CFileItemPtr item;
	
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	if (item && !item->GetPropertyBOOL("pluginreplacecontextitems"))
	{
		if ( m_vecItems->IsVirtualDirectoryRoot() && item)
		{
			CGUIDialogContextMenu::GetContextButtons("pictures", item, buttons);
		}
		else
		{
#if 0 // TODO: OG Xbox stuff to be modded to suit 360
			if (item)
			{
				if (!(item->m_bIsFolder || item->IsZIP() || item->IsRAR() || item->IsCBZ() || item->IsCBR()))
					buttons.Add(CONTEXT_BUTTON_INFO, 13406); // picture info
				
				buttons.Add(CONTEXT_BUTTON_VIEW_SLIDESHOW, item->m_bIsFolder ? 13317 : 13422);      // View Slideshow
				
				if (item->m_bIsFolder)
					buttons.Add(CONTEXT_BUTTON_RECURSIVE_SLIDESHOW, 13318);     // Recursive Slideshow

				if (!m_thumbLoader.IsLoading())
					buttons.Add(CONTEXT_BUTTON_REFRESH_THUMBS, 13315);         // Create Thumbnails
				
				if (g_guiSettings.GetBool("filelists.allowfiledeletion") && !item->IsReadOnly())
				{
					buttons.Add(CONTEXT_BUTTON_DELETE, 117);
					buttons.Add(CONTEXT_BUTTON_RENAME, 118);
				}
			}
			buttons.Add(CONTEXT_BUTTON_GOTO_ROOT, 20128);
			buttons.Add(CONTEXT_BUTTON_SWITCH_MEDIA, 523);
#endif
		}
	}
	
	CGUIMediaWindow::GetContextButtons(itemNumber, buttons);

#if 0 // TODO: OG Xbox stuff to be modded to suit 360
	if (item && !item->GetPropertyBOOL("pluginreplacecontextitems"))
		buttons.Add(CONTEXT_BUTTON_SETTINGS, 5); // Settings
#endif
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
	
#if 0 // TODO: OG Xbox stuff to be modded to suit 360
	switch (button)
	{
		// TODO - Add more context options!

		default:
			break;
	}
#endif
	
	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}
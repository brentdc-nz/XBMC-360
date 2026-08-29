#include "pictures/GUIDialogPictureInfo.h"
#include "guilib/GUIInfoManager.h"
#include "guilib/GUIWindowManager.h"
#include "FileItem.h"
#include "guilib/LocalizeStrings.h"

#define CONTROL_PICTURE_INFO 5

#define SLIDE_STRING_BASE 21800 - SLIDE_INFO_START

CGUIDialogPictureInfo::CGUIDialogPictureInfo(void)
	: CGUIDialog(WINDOW_DIALOG_PICTURE_INFO, "DialogPictureInfo.xml")
{
	m_pictureInfo = new CFileItemList;
}

CGUIDialogPictureInfo::~CGUIDialogPictureInfo(void)
{
	delete m_pictureInfo;
}

void CGUIDialogPictureInfo::SetPicture(CFileItem *item)
{
	g_infoManager.SetCurrentSlide(*item);
}

void CGUIDialogPictureInfo::OnInitWindow()
{
	CGUIDialog::OnInitWindow();
	UpdatePictureInfo();
}

bool CGUIDialogPictureInfo::OnAction(const CAction& action)
{
	switch (action.GetID())
	{
		// If we're running from slideshow mode, drop the "next picture" and "previous picture" actions through.
		case ACTION_NEXT_PICTURE:
		case ACTION_PREV_PICTURE:
		case ACTION_PLAYER_PLAY:
		case ACTION_PAUSE:
			if (g_windowManager.GetActiveWindow() == WINDOW_SLIDESHOW)
			{
				CGUIWindow* pWindow = g_windowManager.GetWindow(WINDOW_SLIDESHOW);
				return pWindow->OnAction(action);
			}
			break;
	};
	return CGUIDialog::OnAction(action);
}

void CGUIDialogPictureInfo::FrameMove()
{
	if (g_infoManager.GetCurrentSlide().GetPath() != m_currentPicture)
	{
		UpdatePictureInfo();
		m_currentPicture = g_infoManager.GetCurrentSlide().GetPath();
	}

	CGUIDialog::FrameMove();
}

void CGUIDialogPictureInfo::UpdatePictureInfo()
{
	// add stuff from the current slide to the list
	CGUIMessage msgReset(GUI_MSG_LABEL_RESET, GetID(), CONTROL_PICTURE_INFO);
	OnMessage(msgReset);
	m_pictureInfo->Clear();

	for (int info = SLIDE_INFO_START; info <= SLIDE_INFO_END; ++info)
	{
		CStdString picInfo = g_infoManager.GetLabel(info);
		
		if (!picInfo.IsEmpty())
		{
			CFileItemPtr item(new CFileItem(g_localizeStrings.Get(SLIDE_STRING_BASE + info)));
			item->SetLabel2(picInfo);
			m_pictureInfo->Add(item);
		}
	}

	CGUIMessage msg(GUI_MSG_LABEL_BIND, GetID(), CONTROL_PICTURE_INFO, 0, 0, m_pictureInfo);
	OnMessage(msg);
}

void CGUIDialogPictureInfo::OnDeinitWindow(int nextWindowID)
{
	CGUIDialog::OnDeinitWindow(nextWindowID);
	CGUIMessage msgReset(GUI_MSG_LABEL_RESET, GetID(), CONTROL_PICTURE_INFO);
	OnMessage(msgReset);
	m_pictureInfo->Clear();
	m_currentPicture.Empty();
}

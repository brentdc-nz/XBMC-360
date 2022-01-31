#include "GUIDialogMediaSource.h"
#include "..\GUIWindowManager.h"
#include "..\LocalizeStrings.h"
#include "GUIDialogFileBrowser.h"
#include "..\..\MediaManager.h"
#include "..\..\Application.h"
#include "..\..\URL.h"
#include "..\..\utils\Util.h"
#include "..\..\filesystem\Directory.h"

#define CONTROL_HEADING         2
#define CONTROL_PATH            10
#define CONTROL_PATH_BROWSE     11
#define CONTROL_NAME            12
#define CONTROL_PATH_ADD        13
#define CONTROL_PATH_REMOVE     14
#define CONTROL_OK              18
#define CONTROL_CANCEL          19

using namespace DIRECTORY;

CGUIDialogMediaSource::CGUIDialogMediaSource(void)
    : CGUIDialog(WINDOW_DIALOG_MEDIA_SOURCE, "DialogMediaSource.xml")
{
}

CGUIDialogMediaSource::~CGUIDialogMediaSource()
{
}

bool CGUIDialogMediaSource::OnAction(const CAction &action)
{
	if(action.GetID()== ACTION_PREVIOUS_MENU)
		m_confirmed = false;

	return CGUIDialog::OnAction(action);
}

bool CGUIDialogMediaSource::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();
			int iAction = message.GetParam1();
			
			if(iControl == CONTROL_PATH_BROWSE)
				OnPathBrowse(GetSelectedItem());
			else if(iControl == CONTROL_PATH_ADD)
				OnPathAdd();
			else if(iControl == CONTROL_PATH_REMOVE)
				OnPathRemove(GetSelectedItem());
			else if(iControl == CONTROL_OK)
				OnOK();
			else if(iControl == CONTROL_CANCEL)
				OnCancel();

			return true;
		}
		break;
	}

	return CGUIDialog::OnMessage(message);
}

// Show CGUIDialogMediaSource dialog and prompt for a new media source.
// return True if the media source is added, false otherwise.
bool CGUIDialogMediaSource::ShowAndAddMediaSource(const CStdString &type)
{
	CGUIDialogMediaSource *dialog = (CGUIDialogMediaSource *)g_windowManager.GetWindow(WINDOW_DIALOG_MEDIA_SOURCE);
	if(!dialog) return false;

	dialog->Initialize();
	dialog->SetShare(CMediaSource());
	dialog->SetTypeOfMedia(type);
	dialog->DoModal();

	g_application.FrameMove(); // FIXME: Very nasty hack!

	bool bConfirmed(dialog->IsConfirmed());
	
	if(bConfirmed)
	{
		// Yay, add this share
		CMediaSource share;
		share.FromNameAndPaths(type, dialog->m_name, dialog->GetPaths());
		g_settings.AddShare(type, share);
	}
	dialog->m_paths.Clear();

	return bConfirmed;
}

void CGUIDialogMediaSource::SetTypeOfMedia(const CStdString &type, bool editNotAdd)
{
	m_type = type;
	int typeStringID = -1;

	if(type == "music")
		typeStringID = 249; // "Music"
	else if(type == "video")
		typeStringID = 291;  // "Video"
	else if(type == "programs")
		typeStringID = 350;  // "Programs"
	else if(type == "pictures")
		typeStringID = 1213;  // "Pictures"
	else // if(type == "files");
		typeStringID = 744;  // "Files"
	
	CStdString format;
	format.Format(g_localizeStrings.Get(editNotAdd ? 1028 : 1020).c_str(), g_localizeStrings.Get(typeStringID).c_str());
	SET_CONTROL_LABEL(CONTROL_HEADING, format);
}

void CGUIDialogMediaSource::SetShare(const CMediaSource &share)
{
	m_paths.Clear();

	for(unsigned int i = 0; i < share.vecPaths.size(); i++)
		m_paths.Add(new CFileItem(share.vecPaths[i], true));

	if(0 == share.vecPaths.size())
		m_paths.Add(new CFileItem("", true));

	m_name = share.strName;

	UpdateButtons();
}

int CGUIDialogMediaSource::GetSelectedItem()
{
	return 0;
}

void CGUIDialogMediaSource::OnPathBrowse(int item)
{
	if(item < 0 || item > m_paths.Size()) return;

	// Browse is called.  Open the filebrowser dialog.
	// Ignore current path is best at this stage??
	CStdString path;
	bool allowNetworkShares(m_type != "myprograms");

	if(CGUIDialogFileBrowser::ShowAndGetShare(path, allowNetworkShares/*, extraShares.size()==0?NULL:&extraShares*/))
	{
		m_paths[item]->SetPath(path);

		if(m_name.IsEmpty())
		{
			CURL url(path);
			url.GetURLWithoutUserDetails(m_name);
			CUtil::RemoveSlashAtEnd(m_name);
			m_name = CUtil::GetTitleFromPath(m_name);
		} 
		UpdateButtons();
	}
}

void CGUIDialogMediaSource::UpdateButtons()
{
	if(m_paths[0]->m_strPath.IsEmpty() || m_name.IsEmpty())
	{
		CONTROL_DISABLE(CONTROL_OK);
	}
	else
	{
		CONTROL_ENABLE(CONTROL_OK);
	}

	if(m_paths.Size() <= 1)
	{
		CONTROL_DISABLE(CONTROL_PATH_REMOVE);
	}
	else
	{
		CONTROL_ENABLE(CONTROL_PATH_REMOVE);
	}

	// Set name
	SET_CONTROL_LABEL(CONTROL_NAME, m_name);

	if(/*m_hasMultiPath*/1)
	{
		int currentItem = GetSelectedItem();
		CGUIMessage msgReset(GUI_MSG_LABEL_RESET, GetID(), CONTROL_PATH);
		OnMessage(msgReset);
		
		for(int i = 0; i < m_paths.Size(); i++)
		{
			CFileItem* item = m_paths[i];
			CStdString path;
			CURL url(item->GetPath());
			url.GetURLWithoutUserDetails(path);
     
			if(path.IsEmpty())
				path = "<"+g_localizeStrings.Get(231)+">"; // <None>
			
			item->SetLabel(path);
			CGUIMessage msg(GUI_MSG_ITEM_ADD, GetID(), CONTROL_PATH, 0, 0, (void*)item);
			OnMessage(msg);
		}
		CGUIMessage msg(GUI_MSG_ITEM_SELECT, GetID(), CONTROL_PATH, currentItem);
		OnMessage(msg);
	}
	else
	{
		CStdString path;
		CURL url(m_paths[0]->GetPath());
		url.GetURLWithoutUserDetails(path);
		if(path.IsEmpty())
			path = "<"+g_localizeStrings.Get(231)+">"; // <None>
		
		SET_CONTROL_LABEL(CONTROL_PATH, path);
	}
}

void CGUIDialogMediaSource::HighlightItem(int item)
{
	for(int i = 0; i < m_paths.Size(); i++)
		m_paths[i]->Select(false);

	if(item >= 0 && item < m_paths.Size())
		m_paths[item]->Select(true);

	CGUIMessage msg(GUI_MSG_ITEM_SELECT, GetID(), CONTROL_PATH, item);
	OnMessage(msg);
}

void CGUIDialogMediaSource::OnPathAdd()
{
	// Add a new item and select it as well
	m_paths.Add(new CFileItem("", true));
	UpdateButtons();
	HighlightItem(m_paths.Size() - 1);
}

void CGUIDialogMediaSource::OnPathRemove(int item)
{
	m_paths.Remove(item);
	UpdateButtons();
	
	if(item >= m_paths.Size())
		HighlightItem(m_paths.Size() - 1);
	else
		HighlightItem(item);

	if(m_paths.Size() <= 1)
		SET_CONTROL_FOCUS(CONTROL_PATH_ADD, 0);
}

void CGUIDialogMediaSource::OnOK()
{
	// Verify the path by doing a GetDirectory.
	CFileItemList items;

	CMediaSource share;
	share.FromNameAndPaths(m_type, m_name, GetPaths());

	// Hack: Need to temporarily add the share, then get path, then remove share
	VECSOURCES *shares = g_settings.GetSourcesFromType(m_type);
	
	if(shares)
		shares->push_back(share);

	if(CDirectory::GetDirectory(share.strPath, items, "", false, true)/* || CGUIDialogYesNo::ShowAndGetInput(1001,1025,1003,1004)*/)
	{
		m_confirmed = true;
		Close();
	}

	// And remove the share again
	if(shares)
		shares->erase(--shares->end());
}

void CGUIDialogMediaSource::OnCancel()
{
	m_confirmed = false;
	Close();
}

vector<CStdString> CGUIDialogMediaSource::GetPaths()
{
	vector<CStdString> paths;

	for(int i = 0; i < m_paths.Size(); i++)
		paths.push_back(m_paths[i]->GetPath());
	
	return paths;
}
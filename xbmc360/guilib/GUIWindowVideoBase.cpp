#include "GUIWindowVideoBase.h"
#include "video\VideoDatabase.h"
#include "guilib\dialogs\GUIDialogContextMenu.h"
#include "guilib\LocalizeStrings.h"
#include "GUISettings.h"
#include "utils\StringUtils.h"
#include "utils\log.h"
#include "FileItem.h"

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

bool CGUIWindowVideoBase::Update(const CStdString &strDirectory)
{
	if (m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if (!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(*m_vecItems);

	return true;
}

bool CGUIWindowVideoBase::OnClick(int iItem)
{
	if (g_guiSettings.GetInt("videoplayer.resumeautomatically") != RESUME_NO)
		OnResumeItem(iItem);
	else
		return CGUIMediaWindow::OnClick(iItem);

	return true;
}

void CGUIWindowVideoBase::OnRestartItem(int iItem)
{
	CGUIMediaWindow::OnClick(iItem);
}

bool CGUIWindowVideoBase::OnResumeShowMenu(CFileItem &item)
{
	// we always resume the movie if the user doesn't want us to ask
	bool resumeItem = g_guiSettings.GetInt("videoplayer.resumeautomatically") != RESUME_ASK;

	if (!item.m_bIsFolder && !item.IsLiveTV() && !resumeItem)
	{
		// check to see whether we have a resume offset available
		CVideoDatabase db;
		if (db.Open())
		{
			CBookmark bookmark;
			CStdString itemPath(item.GetPath());
			if (db.GetResumeBookMark(itemPath, bookmark))
			{ // prompt user whether they wish to resume
				CContextButtons choices;
				CStdString resumeString;
				resumeString.Format(g_localizeStrings.Get(12022).c_str(), CStringUtils::SecondsToTimeString((long)bookmark.timeInSeconds).c_str());
				choices.Add(1, resumeString);
				choices.Add(2, 12021); // start from the beginning
				int retVal = CGUIDialogContextMenu::ShowAndGetChoice(choices);
				if (retVal < 0)
					return false; // don't do anything
				resumeItem = (retVal == 1);
			}
			db.Close();
		}
	}
	if (resumeItem)
		item.m_lStartOffset = STARTOFFSET_RESUME;

	return true;
}

void CGUIWindowVideoBase::OnResumeItem(int iItem)
{
	if (iItem < 0 || iItem >= m_vecItems->Size()) return;
	CFileItemPtr item = m_vecItems->Get(iItem);

	// Show menu asking the user
	if (OnResumeShowMenu(*item))
		CGUIMediaWindow::OnClick(iItem);
}
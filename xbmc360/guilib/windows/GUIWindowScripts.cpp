#include "GUIWindowScripts.h"
#include "utils\URIUtils.h"
#include "interfaces\python\XBPython.h"
#include "guilib\windows\GUIWindowScriptsInfo.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\windows\GUIWindowFileManager.h"
#include "filesystem\File.h"
#include "FileItem.h"
#include "guilib\LocalizeStrings.h"

using namespace XFILE;

#define CONTROL_BTNVIEWASICONS     2
#define CONTROL_BTNSORTBY          3
#define CONTROL_BTNSORTASC         4
#define CONTROL_LIST              50
#define CONTROL_THUMBS            51
#define CONTROL_LABELFILES        12

CGUIWindowScripts::CGUIWindowScripts()
	: CGUIMediaWindow(WINDOW_SCRIPTS, "MyScripts.xml")
{
	m_bViewOutput = false;
	m_scriptSize = 0;
}

CGUIWindowScripts::~CGUIWindowScripts()
{
}

bool CGUIWindowScripts::OnAction(const CAction &action)
{
	if (action.GetID() == ACTION_SHOW_INFO)
	{
		OnInfo();
		return true;
	}
	return CGUIMediaWindow::OnAction(action);
}

bool CGUIWindowScripts::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
	case GUI_MSG_WINDOW_INIT:
		{
			if (m_vecItems->GetPath() == "?")
				m_vecItems->SetPath("D:\\scripts");

			return CGUIMediaWindow::OnMessage(message);
		}
		break;
	}
	return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowScripts::Update(const CStdString &strDirectory)
{
	// Look if baseclass can handle it
	if (!CGUIMediaWindow::Update(strDirectory))
		return false;

	/* Check if any python scripts are running. If true, place "(Running)" after the item.
	 * since stopping a script can take up to 10 seconds or more, we display 'stopping'
	 * after the filename for now.
	 */
	int iSize = g_pythonParser.ScriptsSize();
	for (int i = 0; i < iSize; i++)
	{
		int id = g_pythonParser.GetPythonScriptId(i);
		if (g_pythonParser.isRunning(id))
		{
			const char* filename = g_pythonParser.getFileName(id);

			for (int j = 0; j < m_vecItems->Size(); j++)
			{
				CFileItemPtr pItem = m_vecItems->Get(j);
				if (pItem->GetPath() == filename)
				{
					CStdString runningLabel = pItem->GetLabel() + " (";
					if (g_pythonParser.isStopping(id))
						runningLabel += g_localizeStrings.Get(23053) + ")";
					else
						runningLabel += g_localizeStrings.Get(23054) + ")";
					pItem->SetLabel(runningLabel);
				}
			}
		}
	}

	return true;
}

bool CGUIWindowScripts::OnPlayMedia(int iItem)
{
	CFileItemPtr pItem = m_vecItems->Get(iItem);
	CStdString strPath = pItem->GetPath();

	/* Execute script...
	 * If script is already running do not run it again but stop it.
	 */
	int id = g_pythonParser.getScriptId(strPath.c_str());

	if (id != -1)
	{
		/* If we are here we already know that this script is running.
		 * But we will check it again to be sure :)
		 */
		if (g_pythonParser.isRunning(id))
		{
			g_pythonParser.stopScript(id);

			// Update items
			int selectedItem = m_viewControl.GetSelectedItem();
			Update(m_vecItems->GetPath());
			m_viewControl.SetSelectedItem(selectedItem);
			return true;
		}
	}
	unsigned int argc = 1;
	char ** argv = new char*[argc];
	argv[0] = (char*)strPath.c_str();
	g_pythonParser.evalFile(argv[0], argc, (const char**)argv);
	delete [] argv;

	return true;
}

void CGUIWindowScripts::OnInfo()
{
	CGUIWindowScriptsInfo* pDlgInfo = (CGUIWindowScriptsInfo*)g_windowManager.GetWindow(WINDOW_SCRIPTS_INFO);
	if (pDlgInfo) pDlgInfo->DoModal();
}

void CGUIWindowScripts::FrameMove()
{
	// Update control_list / control_thumbs if one or more scripts have stopped / started
	if (g_pythonParser.ScriptsSize() != m_scriptSize)
	{
		int selectedItem = m_viewControl.GetSelectedItem();
		Update(m_vecItems->GetPath());
		m_viewControl.SetSelectedItem(selectedItem);
		m_scriptSize = g_pythonParser.ScriptsSize();
	}

	CGUIWindow::FrameMove();
}

bool CGUIWindowScripts::GetDirectory(const CStdString& strDirectory, CFileItemList& items)
{
	if (!CGUIMediaWindow::GetDirectory(strDirectory, items))
		return false;

	// Flatten any folders
	for (int i = 0; i < items.Size(); i++)
	{
		CFileItemPtr item = items[i];
		if (item->m_bIsFolder && !item->IsParentFolder() && !item->m_bIsShareOrDrive && !item->GetLabel().Left(1).Equals("."))
		{
			// Folder item - let's check for a default.py file, and flatten if we have one
			CStdString defaultPY;
			URIUtils::AddFileToFolder(item->GetPath(), "default.py", defaultPY);

			if (CFile::Exists(defaultPY))
			{
				// Yes, format the item up
				item->SetPath(defaultPY);
				item->m_bIsFolder = false;
				item->FillInDefaultIcon();
				item->SetLabelPreformated(true);
			}
		}
		if (item->GetLabel().Equals("autoexec.py") || (item->GetLabel().Left(1).Equals(".") && !item->IsParentFolder()))
		{
			items.Remove(i);
			i--;
		}
	}

	return true;
}

void CGUIWindowScripts::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
	buttons.Add(CONTEXT_BUTTON_INFO, 654);
	buttons.Add(CONTEXT_BUTTON_DELETE, 117);
}

bool CGUIWindowScripts::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	if (button == CONTEXT_BUTTON_INFO)
	{
		OnInfo();
		return true;
	}
	else if (button == CONTEXT_BUTTON_DELETE)
	{
		CStdString path;
		URIUtils::GetDirectory(m_vecItems->Get(itemNumber)->GetPath(), path);
		CFileItem item2(path, true);
		if (CGUIWindowFileManager::DeleteItem(&item2))
			Update(m_vecItems->GetPath());

		return true;
	}
	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

#include "MultiPathDirectory.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"
#include "Directory.h"

using namespace XFILE;

//
// Multipath://{path1} , {path2} , {path3} , ... , {path-N}
//
// Unlike the older virtualpath:// protocol, sub-folders are combined together into a new
// multipath:// style url.
//

CMultiPathDirectory::CMultiPathDirectory()
{
}

CMultiPathDirectory::~CMultiPathDirectory()
{
}

bool CMultiPathDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
	CLog::Log(LOGDEBUG,"CMultiPathDirectory::GetDirectory(%s)", strPath.c_str());
#if 0
	vector<CStdString> vecPaths;

	if(!GetPaths(strPath, vecPaths))
		return false;

/*
	DWORD progressTime = timeGetTime() + 3000L; // Three seconds before showing progress bar
	CGUIDialogProgress* dlgProgress = NULL;
*/
	int iFailures = 0;
	for(int i = 0; i < (int)vecPaths.size(); ++i)
	{
		// Show the progress dialog if we have passed our time limit
/*		if(timeGetTime() > progressTime && !dlgProgress)
		{
			dlgProgress = (CGUIDialogProgress *)m_gWindowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
			if(dlgProgress)
			{
				dlgProgress->SetHeading(15310);
				dlgProgress->SetLine(0, 15311);
				dlgProgress->SetLine(1, "");
				dlgProgress->SetLine(2, "");
				dlgProgress->StartModal();
				dlgProgress->ShowProgressBar(true);
				dlgProgress->SetProgressMax((int)vecPaths.size()*2);
				dlgProgress->Progress();
			}
		}

		if(dlgProgress)
		{
			CURL url(vecPaths[i]);
			CStdString strStripped;
			url.GetURLWithoutUserDetails(strStripped);
			dlgProgress->SetLine(1, strStripped);
			dlgProgress->SetProgressAdvance();
			dlgProgress->Progress();
		}
*/
		CFileItemList tempItems;
		CLog::Log(LOGDEBUG,"Getting Directory (%s)", vecPaths[i].c_str());
		
		if(CDirectory::GetDirectory(vecPaths[i], tempItems, m_strFileMask))
			items.Append(tempItems);
		else
		{
			CLog::Log(LOGERROR,"Error Getting Directory (%s)", vecPaths[i].c_str());
			iFailures++;
		}
/*
		if(dlgProgress)
		{
			dlgProgress->SetProgressAdvance();
			dlgProgress->Progress();
		}
*/	}

//	if(dlgProgress)
//		dlgProgress->Close();

	if(iFailures == vecPaths.size())
		return false;

	// Merge like-named folders into a sub multipath:// style url
	MergeItems(items);
#endif
	return true;
}
#if 0
bool CMultiPathDirectory::GetPaths(const CStdString& strPath, vector<CStdString>& vecPaths)
{
	vecPaths.empty();
	CStdString strPath1 = strPath;

	// Remove multipath:// from path
	strPath1 = strPath1.Mid(12);

	// Split on " , "
	vector<CStdString> vecTemp;
	CStringUtils::SplitString(strPath1, " , ", vecTemp);

	if(vecTemp.size() == 0)
		return false;

	// Check each item
	for(int i = 0; i < (int)vecTemp.size(); i++)
	{
		CStdString tempPath = vecTemp[i];

		// Replace double comma's with single ones.
		tempPath.Replace(",,", ",");
		vecPaths.push_back(tempPath);
	}
	
	if(vecPaths.size() == 0)
		return false;

	return true;
}
#endif
CStdString CMultiPathDirectory::GetFirstPath(const CStdString &strPath)
{
	int pos = strPath.Find(" , ", 12);

	if(pos >= 0)
	{
		CStdString firstPath = strPath.Mid(12, pos - 12);
		firstPath.Replace(",,",",");
		return firstPath;
	}
	return "";
}

void CMultiPathDirectory::MergeItems(CFileItemList &items)
{
	CLog::Log(LOGDEBUG, "CMultiPathDirectory::MergeItems, items = %i", (int)items.Size());
	DWORD dwTime=GetTickCount();

	CLog::Log(LOGDEBUG, "CMultiPathDirectory::MergeItems, items = %i,  took %ld ms", items.Size(), GetTickCount()-dwTime);
}

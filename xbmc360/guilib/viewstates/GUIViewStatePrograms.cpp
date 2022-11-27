#include "GUIViewStatePrograms.h"
#include "FileItem.h"
#include "ViewState.h"
#include "Settings.h"
#include "guilib\key.h"

CGUIViewStateWindowPrograms::CGUIViewStateWindowPrograms(const CFileItemList& items) : CGUIViewState(items)
{
	if (/*g_guiSettings.GetBool("filelists.ignorethewhensorting")*/1) // TODO
		AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
	else
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
	
	AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%K", "%J", "%L", "%J"));  // Titel, Date | Foldername, Date
	AddSortMethod(SORT_METHOD_PROGRAM_COUNT, 565, LABEL_MASKS("%K", "%C", "%L", ""));  // Titel, Count | Foldername, empty
	AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%K", "%I", "%K", "%I"));  // Filename, Size | Foldername, Size
	AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | FolderName, empty

	SetSortMethod(/*g_settings.m_viewStatePrograms.m_sortMethod*/SORT_METHOD_LABEL); // TODO
	SetViewAsControl(/*g_settings.m_viewStatePrograms.m_viewMode*/DEFAULT_VIEW_LIST); // TODO
	SetSortOrder(/*g_settings.m_viewStatePrograms.m_sortOrder*/SORT_ORDER_DESC); // TODO

	LoadViewState(items.GetPath(), WINDOW_PROGRAMS);
}

void CGUIViewStateWindowPrograms::SaveViewState()
{
//	SaveViewToDb(m_items.GetPath(), WINDOW_PROGRAMS, &g_settings.m_viewStatePrograms); // TODO
}

CStdString CGUIViewStateWindowPrograms::GetLockType()
{
	return "programs";
}

CStdString CGUIViewStateWindowPrograms::GetExtensions()
{
	return ".xex|.cut";
}

VECSOURCES& CGUIViewStateWindowPrograms::GetSources() // TODO - Plugin share
{
	// Plugins share
/*	if (CPluginDirectory::HasPlugins("programs"))
	{
		CMediaSource share;
		share.strName = g_localizeStrings.Get(1043); // Program Plugins
		share.strPath = "plugin://programs/";
		share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultProgramPlugins.png");
		share.m_ignore= true;
		AddOrReplace(g_settings.m_programSources,share);
	}
*/	return g_settings.m_programSources; 
}
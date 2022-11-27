#include "GUIViewStateVideo.h"
#include "Settings.h"
#include "ViewState.h"
#include "guilib\key.h"

CStdString CGUIViewStateWindowVideo::GetLockType()
{
	return "video";
}

bool CGUIViewStateWindowVideo::UnrollArchives()
{
	return false;//g_guiSettings.GetBool("filelists.unrollarchives"); // TODO
}

CStdString CGUIViewStateWindowVideo::GetExtensions()
{
	return g_settings.GetVideoExtensions();
}

int CGUIViewStateWindowVideo::GetPlaylist()
{
	return 0;//PLAYLIST_VIDEO; // TODO
}

CGUIViewStateWindowVideoFiles::CGUIViewStateWindowVideoFiles(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
	if (items.IsVirtualDirectoryRoot())
	{
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS()); // Preformated
		AddSortMethod(SORT_METHOD_DRIVE_TYPE, 564, LABEL_MASKS()); // Preformated
		SetSortMethod(SORT_METHOD_LABEL);

		SetViewAsControl(DEFAULT_VIEW_ICONS);

		SetSortOrder(SORT_ORDER_ASC);
	}
	else
	{
		if (/*g_guiSettings.GetBool("filelists.ignorethewhensorting")*/1)// TEST
			AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%L", "%I", "%L", "")); // FileName, Size | Foldername, empty
		else
			AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // FileName, Size | Foldername, empty
		
		AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%L", "%I", "%L", "%I")); // FileName, Size | Foldername, Size
		AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%L", "%J", "%L", "%J")); // FileName, Date | Foldername, Date
		AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | FolderName, empty

		SetSortMethod(/*g_settings.m_viewStateVideoFiles.m_sortMethod*/SORT_METHOD_LABEL); // TODO
		SetViewAsControl(/*g_settings.m_viewStateVideoFiles.m_viewMode*/DEFAULT_VIEW_LIST); // TODO
		SetSortOrder(/*g_settings.m_viewStateVideoFiles.m_sortOrder*/SORT_ORDER_DESC); // TODO
	}
	LoadViewState(items.GetPath(), WINDOW_VIDEO_FILES);
}

void CGUIViewStateWindowVideoFiles::SaveViewState()
{
	int itest = 1;
   // SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_FILES, &g_settings.m_viewStateVideoFiles); // TODO
}

VECSOURCES& CGUIViewStateWindowVideoFiles::GetSources() // TODO - Plugin share
{
/*	// Plugins share
	if (CPluginDirectory::HasPlugins("video") && g_advancedSettings.m_bVirtualShares)
	{
		CMediaSource share;
		share.strName = g_localizeStrings.Get(1037);
		share.strPath = "plugin://video/";
		share.m_ignore = true;
		AddOrReplace(g_settings.m_videoSources,share);
	}
*/	return g_settings.m_videoSources; 
}
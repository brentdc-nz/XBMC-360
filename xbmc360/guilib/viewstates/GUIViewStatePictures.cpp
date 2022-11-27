#include "GUIViewStatePictures.h"
#include "FileItem.h"
#include "ViewState.h"
#include "Settings.h"
#include "guilib\key.h"

CGUIViewStateWindowPictures::CGUIViewStateWindowPictures(const CFileItemList& items) : CGUIViewState(items)
{
	if (items.IsVirtualDirectoryRoot())
	{
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS());
		AddSortMethod(SORT_METHOD_DRIVE_TYPE, 564, LABEL_MASKS());
		SetSortMethod(SORT_METHOD_LABEL);

		SetViewAsControl(DEFAULT_VIEW_LIST);

		SetSortOrder(SORT_ORDER_ASC);
	}
	else
	{
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | Foldername, empty
		AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));  // Filename, Size | Foldername, Size
		AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));  // Filename, Date | Foldername, Date
		AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | FolderName, empty

		SetSortMethod(/*g_settings.m_viewStatePictures.m_sortMethod*/SORT_METHOD_LABEL); // TODO
		SetViewAsControl(/*g_settings.m_viewStatePictures.m_viewMode*/DEFAULT_VIEW_LIST); // TODO
		SetSortOrder(/*g_settings.m_viewStatePictures.m_sortOrder*/SORT_ORDER_DESC); // TODO
	}
	LoadViewState(items.GetPath(), WINDOW_PICTURES);
}

void CGUIViewStateWindowPictures::SaveViewState()
{
//	SaveViewToDb(m_items.GetPath(), WINDOW_PICTURES, &g_settings.m_viewStatePictures); // TODO
}

CStdString CGUIViewStateWindowPictures::GetLockType()
{
	return "pictures";
}

bool CGUIViewStateWindowPictures::UnrollArchives()
{
	return false;//g_guiSettings.GetBool("filelists.unrollarchives"); // TODO
}

CStdString CGUIViewStateWindowPictures::GetExtensions()
{
	if (/*g_guiSettings.GetBool("pictures.showvideos")*/0) // TODO
		return g_settings.GetPictureExtensions()+"|"+g_settings.GetVideoExtensions();

	return g_settings.GetPictureExtensions();
}

VECSOURCES& CGUIViewStateWindowPictures::GetSources() // TODO - Plugin share
{
	// Plugins share
/*	if (CPluginDirectory::HasPlugins("pictures") && g_advancedSettings.m_bVirtualShares)
	{
		CMediaSource share;
		share.strName = g_localizeStrings.Get(1039); // Picture Plugins
		share.strPath = "plugin://pictures/";
		share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultPicturePlugins.png");
		share.m_ignore = true;
		AddOrReplace(g_settings.m_pictureSources,share);
	}
*/	return g_settings.m_pictureSources; 
}
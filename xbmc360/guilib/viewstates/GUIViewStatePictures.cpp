#include "GUIViewStatePictures.h"
#include "..\..\Settings.h"

CGUIViewStateWindowPictures::CGUIViewStateWindowPictures(const CFileItemList& items) : CGUIViewState(items) // TODO : Check same as the real version
{
/*
	if(g_guiSettings.GetBool("filelists.ignorethewhensorting"))
		AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 103, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
	else
		AddSortMethod(SORT_METHOD_LABEL, 103, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
		AddSortMethod(SORT_METHOD_DATE, 104, LABEL_MASKS("%K", "%J", "%L", "%J"));  // Titel, Date | Foldername, Date
		AddSortMethod(SORT_METHOD_PROGRAM_COUNT, 507, LABEL_MASKS("%K", "%C", "%L", ""));  // Titel, Count | Foldername, empty
		AddSortMethod(SORT_METHOD_SIZE, 105, LABEL_MASKS("%K", "%I", "%K", "%I"));  // Filename, Size | Foldername, Size
		SetSortMethod(g_stSettings.m_MyProgramsSortMethod);
*/
	AddViewAsControl(VIEW_METHOD_LIST, 101);
	AddViewAsControl(VIEW_METHOD_THUMBS, 100);

//	SetViewAsControl(g_stSettings.m_MyProgramsViewMethod);

//	SetSortOrder(g_stSettings.m_MyProgramsSortOrder);
}

VECSOURCES& CGUIViewStateWindowPictures::GetShares()
{
	return *g_settings.GetSourcesFromType("pictures");
}

CStdString CGUIViewStateWindowPictures::GetExtensions()
{
	return g_settings.GetPictureExtensions();
}
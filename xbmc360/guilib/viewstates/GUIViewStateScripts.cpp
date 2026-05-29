#include "GUIViewStateScripts.h"
#include "FileItem.h"
#include "ViewState.h"
#include "Settings.h"
#include "guilib\Key.h"

CGUIViewStateWindowScripts::CGUIViewStateWindowScripts(const CFileItemList& items) : CGUIViewState(items)
{
	AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | Foldername, empty
	AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));  // Filename, Date | Foldername, Date
	AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));  // Filename, Size | Foldername, Size
	AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | FolderName, empty
	SetSortMethod(SORT_METHOD_LABEL);

	SetViewAsControl(DEFAULT_VIEW_LIST);
	SetSortOrder(SORT_ORDER_ASC);

	LoadViewState(items.GetPath(), WINDOW_SCRIPTS);
}

void CGUIViewStateWindowScripts::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_SCRIPTS);
}

CStdString CGUIViewStateWindowScripts::GetExtensions()
{
	return ".py";
}

VECSOURCES& CGUIViewStateWindowScripts::GetSources()
{
	m_sources.clear();

	CMediaSource share;
	share.strName = "Scripts";
	share.strPath = "D:\\scripts";
	share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
	m_sources.push_back(share);

	return CGUIViewState::GetSources();
}

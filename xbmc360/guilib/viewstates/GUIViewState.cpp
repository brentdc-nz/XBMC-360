#include "GUIViewState.h"
#include "..\GUIWindowManager.h"
#include "GUIViewStatePrograms.h"
#include "GUIViewStateVideos.h"
#include "GUIViewStateMusic.h"
#include "GUIViewStatePictures.h"

VECSOURCES CGUIViewState::m_shares;

CGUIViewState* CGUIViewState::GetViewState(int windowId, const CFileItemList& items)
{
	if(windowId == 0)
		return GetViewState(g_windowManager.GetActiveWindow(), items);

	if(windowId == WINDOW_PROGRAMS)
		return new CGUIViewStateWindowPrograms(items);

	if(windowId == WINDOW_VIDEOS)
		return new CGUIViewStateWindowVideos(items);

	if(windowId == WINDOW_MUSIC)
		return new CGUIViewStateWindowMusic(items);

	if(windowId == WINDOW_PICTURES)
		return new CGUIViewStateWindowPictures(items);

	// Use as a fallback/default
	return new CGUIViewStateGeneral(items);
}

CGUIViewState::CGUIViewState(const CFileItemList& items) : m_items(items)
{
	m_currentViewAsControl = 0;
}

CGUIViewState::~CGUIViewState()
{
}

VIEW_METHOD CGUIViewState::GetViewAsControl() const
{
	if(m_currentViewAsControl >= 0 && m_currentViewAsControl<(int)m_viewAsControls.size())
		return m_viewAsControls[m_currentViewAsControl].m_viewAsControl;

	return VIEW_METHOD_LIST;
}

void CGUIViewState::AddViewAsControl(VIEW_METHOD viewAsControl, int buttonLabel)
{
	VIEW view;
	view.m_viewAsControl = viewAsControl;
	view.m_buttonLabel = buttonLabel;

	m_viewAsControls.push_back(view);
}

void CGUIViewState::SetViewAsControl(VIEW_METHOD viewAsControl)
{
	for (int i=0; i<(int)m_viewAsControls.size(); ++i)
	{
		if (m_viewAsControls[i].m_viewAsControl == viewAsControl)
		{
			m_currentViewAsControl = i;
			break;
		}
	}
}

VIEW_METHOD CGUIViewState::SetNextViewAsControl()
{
	m_currentViewAsControl++;

	if (m_currentViewAsControl >= (int)m_viewAsControls.size())
		m_currentViewAsControl = 0;

//  SaveViewState(); //TDODO

	return GetViewAsControl();
}

CGUIViewStateGeneral::CGUIViewStateGeneral(const CFileItemList& items) : CGUIViewState(items)
{
//	AddSortMethod(SORT_METHOD_LABEL, 103, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, size | Foldername, empty
//	SetSortMethod(SORT_METHOD_LABEL);

	AddViewAsControl(VIEW_METHOD_LIST, 101);
	AddViewAsControl(VIEW_METHOD_THUMBS, 100);
	SetViewAsControl(VIEW_METHOD_LIST);

//	SetSortOrder(SORT_ORDER_ASC);
}

VECSOURCES& CGUIViewState::GetShares()
{
	return m_shares;
}

CStdString CGUIViewState::GetExtensions()
{
	return "";
}
#include "include.h"
#include "GUIListLabel.h"
//#include "utils/CharsetConverter.h" // TODO
#include <limits>

CGUIListLabel::CGUIListLabel(int parentID, int controlID, float posX, float posY, float width, float height, const CLabelInfo& labelInfo, const CGUIInfoLabel &info, bool alwaysScroll)
	: CGUIControl(parentID, controlID, posX, posY, width, height)
	, m_label(posX, posY, width, height, labelInfo, alwaysScroll ? CGUILabel::OVER_FLOW_SCROLL : CGUILabel::OVER_FLOW_TRUNCATE)
{
	m_info = info;
	m_alwaysScroll = alwaysScroll;
	
	// TODO: Remove this "correction"
	if (labelInfo.align & XBFONT_RIGHT)
		m_label.SetMaxRect(m_posX - m_width, m_posY, m_width, m_height);
	else if (labelInfo.align & XBFONT_CENTER_X)
		m_label.SetMaxRect(m_posX - m_width*0.5f, m_posY, m_width, m_height);
	
	if (m_info.IsConstant())
		SetLabel(m_info.GetLabel(m_parentID, true));
	
	ControlType = GUICONTROL_LISTLABEL;
}

CGUIListLabel::~CGUIListLabel(void)
{
}

void CGUIListLabel::SetScrolling(bool scrolling)
{
	m_label.SetScrolling(scrolling || m_alwaysScroll);
}

void CGUIListLabel::SetSelected(bool selected)
{
	m_label.SetColor(selected ? CGUILabel::COLOR_SELECTED : CGUILabel::COLOR_TEXT);
}

void CGUIListLabel::SetFocus(bool focus)
{
	CGUIControl::SetFocus(focus);

	if (!focus)
		SetScrolling(false);
}

void CGUIListLabel::UpdateColors()
{
	m_label.UpdateColors();
	CGUIControl::UpdateColors();
}

void CGUIListLabel::Render()
{
	m_label.Render();
	CGUIControl::Render();
}

void CGUIListLabel::UpdateInfo(const CGUIListItem *item) // TODO
{
//	if (m_info.IsConstant()) // FIXME ASAP
//		return; // Nothing to do

	if (item)
		SetLabel(m_info.GetItemLabel(item));
	else
		SetLabel(m_info.GetLabel(m_parentID, true));
}

void CGUIListLabel::SetInvalid()
{
	m_label.SetInvalid();

	CGUIControl::SetInvalid();
}

void CGUIListLabel::SetLabel(const CStdString &label)
{
	m_label.SetText(label);
}
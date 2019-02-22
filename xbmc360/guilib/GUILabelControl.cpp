/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUILabelControl.h"

using namespace std;

CGUILabelControl::CGUILabelControl(int parentID, int controlID, float posX, float posY, float width, float height, const CLabelInfo& labelInfo, bool wrapMultiLine, bool bHasPath)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_label(posX, posY, width, height, labelInfo/*, wrapMultiLine ? CGUILabel::OVER_FLOW_WRAP : CGUILabel::OVER_FLOW_TRUNCATE*/) //MARTY FIXME WIP
{
}

CGUILabelControl::~CGUILabelControl(void)
{
}

void CGUILabelControl::Render()
{
	if (!IsVisible()) return;

	if(!m_bAllocated) return;

	m_label.Render();

	m_label.SetText(m_infoLabel.GetLabel(m_parentID));

	CGUIControl::Render();
}

bool CGUILabelControl::OnMessage(CGUIMessage& message)
{
	if (message.GetControlId() == GetID())
	{
		if (message.GetMessage() == GUI_MSG_LABEL_SET)
		{
			SetLabel(message.GetLabel());
			return true;
		}
	}

	return CGUIControl::OnMessage(message);
}

void CGUILabelControl::SetLabel(CStdString strText)
{
	m_infoLabel.SetLabel(strText, "", GetParentID());
}

void CGUILabelControl::SetInfo(const CGUIInfoLabel &infoLabel)
{
	m_infoLabel = infoLabel;
}
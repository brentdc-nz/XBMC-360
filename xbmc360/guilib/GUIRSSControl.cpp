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

#include "include.h"
#include "GUIRSSControl.h"
#include "GUIWindowManager.h"
#include "GUISettings.h"
#include "utils\CriticalSection.h"
#include "utils\SingleLock.h"
// TODO: #include "utils\RssReader.h" - Not yet ported
#include "utils\StringUtils.h"

using namespace std;

CGUIRSSControl::CGUIRSSControl(int parentID, int controlID, float posX, float posY, float width, float height, const CLabelInfo& labelInfo, const CGUIInfoColor &channelColor, const CGUIInfoColor &headlineColor, CStdString& strRSSTags)
: CGUIControl(parentID, controlID, posX, posY, width, height),
  m_scrollInfo(0,0,labelInfo.scrollSpeed,"")
{
	m_label = labelInfo;
	m_headlineColor = headlineColor;
	m_channelColor = channelColor;

	m_strRSSTags = strRSSTags;

	m_pReader = NULL;
	m_rtl = false;
	ControlType = GUICONTROL_RSS;
}

CGUIRSSControl::CGUIRSSControl(const CGUIRSSControl &from)
: CGUIControl(from),m_scrollInfo(from.m_scrollInfo)
{
	m_label = from.m_label;
	m_headlineColor = from.m_headlineColor;
	m_channelColor = from.m_channelColor;
	m_strRSSTags = from.m_strRSSTags;
	m_pReader = NULL;
	ControlType = GUICONTROL_RSS;
}

CGUIRSSControl::~CGUIRSSControl(void)
{
	CSingleLock lock(m_criticalSection);
	if (m_pReader)
	{
		// TODO: m_pReader->SetObserver(NULL); - Not yet ported
	}
	m_pReader = NULL;
}

void CGUIRSSControl::SetUrls(const vector<string> &vecUrl, bool rtl)
{
	m_vecUrls = vecUrl;
	m_rtl = rtl;
	if (m_scrollInfo.pixelSpeed > 0 && rtl)
		m_scrollInfo.pixelSpeed *= -1;
	else if (m_scrollInfo.pixelSpeed < 0 && !rtl)
		m_scrollInfo.pixelSpeed *= -1;
}

void CGUIRSSControl::SetIntervals(const vector<int>& vecIntervals)
{
	m_vecIntervals = vecIntervals;
}

void CGUIRSSControl::UpdateColors()
{
	m_label.UpdateColors();
	m_headlineColor.Update();
	m_channelColor.Update();
	CGUIControl::UpdateColors();
}

void CGUIRSSControl::Render()
{
	// TODO: RSS rendering requires RssReader/RssManager which are not yet ported
	// When ported, this should match the xbmc4xbox implementation
	CGUIControl::Render();
}

void CGUIRSSControl::OnFeedUpdate(const vecText &feed)
{
	CSingleLock lock(m_criticalSection);
	m_feed = feed;
}

void CGUIRSSControl::OnFeedRelease()
{
	m_pReader = NULL;
}

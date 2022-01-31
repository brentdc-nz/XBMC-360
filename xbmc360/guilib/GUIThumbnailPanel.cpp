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

#include "GUIThumbnailPanel.h"
#include "GUIImage.h"
#include "GUIWindowManager.h"
#include "GUILabelControl.h"
#include "utils\StringUtils.h"
#include "GUIWindowManager.h"
#include "GUIScrollBarControl.h"

CGUIThumbnailPanel::CGUIThumbnailPanel(int parentID, int controlID, float posX, float posY, float width, float height, float thumbWidth, float thumbHeight, float thumbTexWidth, float thumbTexHeight, const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus, const CLabelInfo& labelInfo)
	: CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_imgFocus(posX, posY, thumbTexWidth, thumbTexHeight, textureFocus)
    , m_imgNoFocus(posX, posY, thumbTexWidth, thumbTexHeight, textureNoFocus)
{
	m_iColumns = 0;
	m_iRows = 0;

	m_iOffset = 0;
	m_iCursorX = 0;
	m_iCursorY = 0;

	m_iPage = 0;
	m_iTotalPages = 0;

	m_bScrollUp = false;
	m_bScrollDown = false;

	m_iScrollCounter = 0;

	m_label = labelInfo;

	m_strSuffix = L"|";
	CStringUtils::StringtoWString(" "+labelInfo.scrollSuffix, m_strSuffix); // HACK: Needed whitespace before suffix

	m_iItemWidth = (int)thumbWidth;
	m_iItemHeight = (int)thumbHeight;

	m_fThumbTexWidth = thumbTexWidth;
	m_fThumbTexHeight = thumbTexHeight;

	m_iScrollBar = 0;
	m_iObjectCounterLabel = -1;
}

CGUIThumbnailPanel::~CGUIThumbnailPanel(void)
{
}

void CGUIThumbnailPanel::RenderItem(bool bFocus, float fPosX, float fPosY, CGUIListItem* pItem, int iStage)
{
	float fTextPosY = fPosY + m_fThumbTexHeight;
	float fCenteredPosX = fPosX + (m_iItemWidth - m_fThumbTexWidth) / 2;

	if(iStage == 0) // Render images
	{
		if(bFocus && HasFocus()/* && m_iSelect == CONTROL_LIST*/)
		{
			m_imgFocus.SetPosition(fCenteredPosX, fPosY);

			if(/*m_bShowTexture*/1)
				m_imgFocus.Render();
		}
		else
		{
			m_imgNoFocus.SetPosition(fCenteredPosX, fPosY);

			if(/*m_bShowTexture*/1)
				m_imgNoFocus.Render();
		}
		
		CStdString strThumb = pItem->GetThumbnailImage();
		
		if(strThumb.IsEmpty() && pItem->HasIcon())
		{
			// No thumbnail, but it does have an icon
			strThumb = pItem->GetIconImage();
			strThumb.Insert(strThumb.Find("."), "Big");
		}
	
		if(!strThumb.IsEmpty())
		{
			CGUIImage *thumb = pItem->GetThumbnail();
			if(!thumb)
			{
				CTextureInfo TInfo;
				TInfo.filename = strThumb;
				TInfo.useLarge = true;
				TInfo.orientation = 0;

				thumb = new CGUIImage(0, 0, /*m_iThumbXPos +*/ fCenteredPosX, /*m_iThumbYPos +*/ fPosY, /*m_iThumbWidth*/m_fThumbTexWidth, /*m_iThumbHeight*/m_fThumbTexHeight, TInfo);
//				thumb->SetAspectRatio(m_aspectRatio);
				pItem->SetThumbnail(thumb);
			}

			if(thumb)
			{
				int xOff = 0;//((m_iThumbWidth - pImage->GetRenderWidth()) / 2);
				int yOff = 0;//((m_iThumbHeight - pImage->GetRenderHeight()) / 2);
			
				// Only supports center yet, 0 is default meaning use x/y position
//				if(m_iThumbAlign != 0)
				{
//					xOff += ((m_iTextureWidth - m_iThumbWidth) / 2);
//					yOff += ((m_iTextureHeight - m_iThumbHeight) / 2);
					//if thumbPosX or thumbPosX != 0 the thumb will be bumped off-center
				}
		
				// Set file name to make sure it's always up to date (does nothing if it is)
				thumb->SetFileName(strThumb);
				
				if(!thumb->IsAllocated())
					thumb->AllocResources();
			
				thumb->SetPosition(/*m_iThumbXPos +*/ fCenteredPosX/* + xOff*/,/* m_iThumbYPos + */fPosY/* + yOff*/);
				thumb->Render();

/*				// Add the overlay image
				CGUIImage *overlay = pItem->GetOverlay();
				if(!overlay && pItem->HasOverlay())
				{
					overlay = new CGUIImage(0, 0, 0, 0, 0, 0, pItem->GetOverlayImage(), 0x0);
					overlay->SetAspectRatio(m_aspectRatio);
					overlay->AllocResources();
					pItem->SetOverlay(overlay);
				}
				
				// Render the image
				if(overlay)
				{
					float x, y;
					thumb->GetBottomRight(x, y);
					if(m_usingBigIcons)
					{
						overlay->SetWidth(overlay->GetTextureWidth());
						overlay->SetHeight(overlay->GetTextureWidth());
					}
					else
					{
						float scale = (m_iThumbHeightBig) ? (float)m_iThumbHeight / m_iThumbHeightBig : 1.0f;
						overlay->SetWidth((int)(overlay->GetTextureWidth() * scale));
						overlay->SetHeight((int)(overlay->GetTextureHeight() * scale));
					}
					
					// If we haven't yet rendered, make sure we update our sizing
					if(!overlay->HasRendered())
					overlay->CalculateSize();
					
					overlay->SetPosition((int)x - overlay->GetRenderWidth(), (int)y - overlay->GetRenderHeight());
					overlay->Render();
				}*/
			}
		}
	}

	if(iStage == 1) // Render text
	{
		// Hide filenames in thumbnail panel
//		if(m_labelState == HIDE_ALL)
//			return;
//		if(m_labelState == HIDE_FILES && !pItem->m_bIsFolder)
//			return;
//		if(m_labelState == HIDE_FOLDERS && pItem->m_bIsFolder)
//			return;

		CStdStringW strItemLabelUnicode;
		strItemLabelUnicode = pItem->GetLabel().c_str();
//		g_charsetConverter.utf8ToUTF16(pItem->GetLabel().c_str(), strItemLabelUnicode);

		DWORD dwColor = m_label.dwTextColor;

		if(pItem->IsSelected()) 
			dwColor = m_label.dwSelectedTextColor;

		if(bFocus && HasFocus()/* && m_iSelect == CONTROL_LIST*/)
			RenderText((float)fPosX, (float)fTextPosY, dwColor, (WCHAR*) strItemLabelUnicode.c_str(), true);
		else
			RenderText((float)fPosX, (float)fTextPosY, dwColor, (WCHAR*) strItemLabelUnicode.c_str(), false);
	}
}

void CGUIThumbnailPanel::RenderText(float fPosX, float fPosY, DWORD dwTextColor, WCHAR* wszText, bool bScroll)
{
	static int scroll_pos = 0;
	static int iScrollX = 0;
	static int iLastItem = -1;
	static int iFrames = 0;
	static int iStartFrame = 0;
	float fTextHeight, fTextWidth;

	m_label.font->GetTextExtent(wszText, &fTextWidth, &fTextHeight);
	float fMaxWidth = m_iItemWidth - m_iItemWidth / 10.0f;

	if(!bScroll || fTextWidth <= fMaxWidth)
	{
		m_label.font->DrawTextWidth(fPosX, fPosY, dwTextColor, wszText, fMaxWidth);
		return;
	}
	else
	{
		RECT RCScissorTextRect;

		RCScissorTextRect.left = (LONG)fPosX;
		RCScissorTextRect.right = (LONG)fPosX + (LONG)fMaxWidth - 5;
		RCScissorTextRect.top = (LONG)fPosY;
		RCScissorTextRect.bottom = (LONG)fPosY + 60;

		GRAPHICSCONTEXT_LOCK()

		g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE); 
		g_graphicsContext.Get3DDevice()->SetScissorRect(&RCScissorTextRect);

		GRAPHICSCONTEXT_UNLOCK()

		// Scroll
		WCHAR wszOrgText[1024];
		wcscpy(wszOrgText, wszText);
		wcscat(wszOrgText, m_strSuffix.c_str());

		m_label.font->GetTextExtent(wszOrgText, &fTextWidth, &fTextHeight);
		
		int iItem = m_iCursorX + m_iCursorY * m_iColumns + m_iOffset;

		if(fTextWidth > fMaxWidth)
		{
			fMaxWidth += 50;
			WCHAR szText[1024];
			
			if(iLastItem != iItem)
			{
				scroll_pos = 0;
				iLastItem = iItem;
				iStartFrame = 0;
				iScrollX = 1;
			}

			if(iStartFrame > 25)
			{
				WCHAR wTmp[3];
				if(scroll_pos >= (int)wcslen(wszOrgText) )
					wTmp[0] = L' ';
				else
					wTmp[0] = wszOrgText[scroll_pos];
				
				wTmp[1] = 0;
				float fWidth,fHeight;
				
				m_label.font->GetTextExtent(wTmp, &fWidth, &fHeight);
				
				if(iScrollX >= fWidth)
				{
					++scroll_pos;
					if(scroll_pos > (int)wcslen(wszOrgText))
						scroll_pos = 0;

					iFrames = 0;
					iScrollX = 1;
				}
				else iScrollX++;
					
				int ipos = 0;

				for(int i=0; i < (int)wcslen(wszOrgText); i++)
				{
					if(i+scroll_pos < (int)wcslen(wszOrgText))
						szText[i] = wszOrgText[i+scroll_pos];
					else
					{
						if(ipos == 0) szText[i] = L' ';
						else szText[i] = wszOrgText[ipos-1];
						ipos++;
					}
					
					szText[i+1] = 0;
				}
				
				if(fPosY >= 0.0)
					m_label.font->DrawTextWidth(fPosX - iScrollX, fPosY, dwTextColor, szText, fMaxWidth);			
			}
			else
			{
				iStartFrame++;
			
				if(fPosY >= 0.0)
					m_label.font->DrawTextWidth(fPosX, fPosY, dwTextColor, wszText, fMaxWidth);
			}
		}

		GRAPHICSCONTEXT_LOCK()

		// Restore the main panel scissor rect
		g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE); 
		g_graphicsContext.Get3DDevice()->SetScissorRect(&m_RCScissorRect);

		GRAPHICSCONTEXT_UNLOCK()
	}
}

void CGUIThumbnailPanel::Render()
{
	if(!m_label.font) return;
	if(!IsVisible()) return;

	if(!ValidItem(m_iCursorX, m_iCursorY))
	{
		m_iCursorX = 0;
		m_iCursorY = 0;
	}

	CGUIControl::Render();

	int iScrollYOffset = 0;

	if(m_bScrollDown)
		iScrollYOffset =- (m_iItemHeight - m_iScrollCounter);

	if(m_bScrollUp)
		iScrollYOffset = m_iItemHeight - m_iScrollCounter;

	// Set the scissor test to stop overdraw
	// when scrolling through
	GRAPHICSCONTEXT_LOCK()

	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE); 
	g_graphicsContext.Get3DDevice()->SetScissorRect(&m_RCScissorRect);

	GRAPHICSCONTEXT_UNLOCK()

	for (int iStage = 0; iStage <= 1; iStage++)
    {
		if(m_bScrollUp)
		{
			// Render item on top
			float dwPosY = m_posY - m_iItemHeight + iScrollYOffset;
			m_iOffset -= m_iColumns;
		
			for(int iCol = 0; iCol < m_iColumns; iCol++)
			{
				float dwPosX = m_posX + iCol * m_iItemWidth;
				int iItem = iCol + m_iOffset;
			
				if(iItem >= 0 && iItem < (int)m_vecItems.size())
				{
					CGUIListItem *pItem = m_vecItems[iItem];
					RenderItem(false, dwPosX, dwPosY, pItem, iStage);
				}
			}
			m_iOffset += m_iColumns;
		}

		// Render main panel
		for(int iRow = 0; iRow < m_iRows; iRow++)
		{
			float dwPosY = m_posY + iRow * m_iItemHeight + iScrollYOffset;
			for(int iCol = 0; iCol < m_iColumns; iCol++)
			{
				float dwPosX = m_posX + iCol * m_iItemWidth;
				int iItem = iRow * m_iColumns + iCol + m_iOffset;
			
				if(iItem < (int)m_vecItems.size())
				{
					CGUIListItem *pItem = m_vecItems[iItem];
					bool bFocus = (m_iCursorX == iCol && m_iCursorY == iRow );
					RenderItem(bFocus, dwPosX, dwPosY, pItem, iStage);
				}
			}
		}

		if(m_bScrollDown)
		{
			// Render item on bottom
			float fPosY = m_posY + m_iRows * m_iItemHeight + iScrollYOffset;
		
			for(int iCol = 0; iCol < m_iColumns; iCol++)
			{
				float fPosX = m_posX + iCol * m_iItemWidth;
				int iItem = m_iRows * m_iColumns + iCol + m_iOffset;
			
				if(iItem < (int)m_vecItems.size())
				{
					CGUIListItem *pItem = m_vecItems[iItem];
					RenderItem(false, fPosX, fPosY, pItem, iStage);
				}
			}
		}

		int iFrames = 12;
		int iStep = m_iItemHeight / iFrames;
	
		if(!iStep) iStep = 1;
	
		if(m_bScrollDown)
		{
			m_iScrollCounter -= iStep;
			if(m_iScrollCounter <= 0 )
			{
				m_bScrollDown = false;
				m_iOffset += m_iColumns;
				m_iPage = m_iOffset / (m_iRows * m_iColumns);

				m_iPage += 1;
				UpdateCounterLabel();
			}
		}

		if(m_bScrollUp)
		{
			m_iScrollCounter -= iStep;

			if(m_iScrollCounter <= 0)
			{
				m_bScrollUp = false;
				m_iOffset -= m_iColumns;
				m_iPage = m_iOffset / (m_iRows * m_iColumns);

				m_iPage += 1;
				UpdateCounterLabel();
			}
		}
	}	

	GRAPHICSCONTEXT_LOCK()

	// Now disable the scissor test again
	g_graphicsContext.Get3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE); 

	GRAPHICSCONTEXT_UNLOCK()

	// FIXME: We should not be sending this each frame!
	CGUIMessage msg(GUI_MSG_ITEM_SELECT, 0, m_iScrollBar, m_iOffset);
	g_windowManager.SendMessage(msg);
}

bool CGUIThumbnailPanel::OnMessage(CGUIMessage& message)
{
	if(message.GetControlId() == GetID())
	{
		if(message.GetMessage() == GUI_MSG_ITEM_ADD)
		{
			m_vecItems.push_back((CGUIListItem*) message.GetLPVOID());
			int iItemsPerPage = m_iRows*m_iColumns;
			m_iTotalPages = m_vecItems.size() / iItemsPerPage;
			if(m_vecItems.size() % iItemsPerPage) m_iTotalPages++;

//			m_upDown.SetRange(1,iPages);
//			m_upDown.SetValue(1);
			m_iPage = 1;
			UpdateCounterLabel();
			return true;
		}

		if(message.GetMessage() == GUI_MSG_ITEMS_RESET)
		{
			m_vecItems.erase(m_vecItems.begin(), m_vecItems.end());

			m_iTotalPages = 1;
			m_iPage = 1;
			UpdateCounterLabel();

			m_iCursorX = m_iCursorY = 0;
			// Don't reset our row offset here - it's taken care of in SELECT
			//m_iRowOffset = 0;
			m_bScrollUp = false;
			m_bScrollDown = false;
			return true;
		}

		if(message.GetMessage() == GUI_MSG_LABEL_RESET)
		{
			m_vecItems.erase(m_vecItems.begin(), m_vecItems.end());
//			m_upDown.SetRange(1, 1);
//			m_upDown.SetValue(1);
			m_iCursorX = m_iCursorY = 0;
			// Don't reset our row offset here - it's taken care of in SELECT
			//m_iRowOffset = 0;
			m_bScrollUp = false;
			m_bScrollDown = false;
			return true;
		}

		if(message.GetMessage() == GUI_MSG_ITEM_SELECTED)
		{
			message.SetParam1(m_iOffset + m_iCursorY*m_iColumns+m_iCursorX);
			return true;
		}

		if(message.GetMessage() == GUI_MSG_ITEM_SELECT)
		{
			int iItem=message.GetParam1();

			if(iItem >=0 && iItem < (int)m_vecItems.size())
			{
				m_iPage = 1;
				m_iCursorX = 0;
				m_iCursorY = 0;
				m_iOffset = 0;
				
				while(iItem >= (m_iRows*m_iColumns))
				{
					m_iOffset += (m_iRows*m_iColumns);
					iItem -= (m_iRows*m_iColumns);
					m_iPage++;
				}
				
				while(iItem >= m_iColumns)
				{
					m_iCursorY++;
					iItem -= m_iColumns;
				}

				UpdateCounterLabel();

//				m_upDown.SetValue(iPage);
				m_iCursorX = iItem;
				return true;
			}
		}

		if(message.GetMessage() == GUI_MSG_SCROLL_CHANGE)
		{
			if(message.GetParam1() == GUI_MSG_SCROLL_UP)
				OnPageUp();
			else
				OnPageDown();

			return true;
		}
	}

	return CGUIControl::OnMessage(message);
}

void CGUIThumbnailPanel::AllocResources()
{
	CGUIControl::AllocResources();	

	m_imgFocus.AllocResources();
	m_imgNoFocus.AllocResources();

	float fWidth, fHeight;
  
	// Height of 1 item = folder image height + text row height + space in between
//	m_pFont->GetTextExtent( L"y", &fWidth, &fHeight);
  
  	fWidth = (float)m_iItemWidth;
	fHeight = (float)m_iItemHeight;

	float fTotalHeight = (float)(m_height -5);
	m_iRows = (int)(fTotalHeight / fHeight);
    
  	m_iColumns = (int)(m_width / fWidth);
	int iItemsPerPage = m_iRows * m_iColumns;
	m_iTotalPages = m_vecItems.size() / iItemsPerPage;
	if(m_vecItems.size() % iItemsPerPage) m_iTotalPages++;

	m_iPage = 1;
	UpdateCounterLabel();

	// Setup up our scissor rect
	m_RCScissorRect.left = (LONG)m_posX;
	m_RCScissorRect.right = (LONG)m_posX + (LONG)m_width;
	m_RCScissorRect.top = (LONG)m_posY;
	m_RCScissorRect.bottom = (LONG)m_posY + (LONG)m_height;
}

void CGUIThumbnailPanel::SetScrollbarControl(const int iControl)
{
	m_iScrollBar = iControl;
}

void CGUIThumbnailPanel::SetObjectLabelControl(const int iControl)
{
	m_iObjectCounterLabel = iControl;
}

bool CGUIThumbnailPanel::ValidItem(int iX, int iY)
{
	if(iX >= m_iColumns) return false;
	if(iY >= m_iRows) return false;
	if(m_iOffset + iY * m_iColumns + iX < (int)m_vecItems.size()) return true;

	return false;
}

void CGUIThumbnailPanel::UpdateCounterLabel()
{
	// Update our counter label
	const CGUILabelControl* pLabelControl = NULL;
	pLabelControl = (CGUILabelControl*)g_windowManager.GetWindow(g_windowManager.GetActiveWindow())->GetControl(m_iObjectCounterLabel);
	
	if(pLabelControl && (m_iObjectCounterLabel != -1))
	{
		CStdString strTemp;
		strTemp.Format("%i Objects - Pages %i / %i", m_vecItems.size(), m_iPage, m_iTotalPages); // TODO: Call localization object

		CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), pLabelControl->GetID());
		msg.SetLabel(strTemp);
		g_windowManager.SendMessage(msg);
	}

	// TODO: Move setting the range to the correct location
	CGUIScrollBar* pScrollbarControl = NULL;
	pScrollbarControl = (CGUIScrollBar*)g_windowManager.GetWindow(g_windowManager.GetActiveWindow())->GetControl(m_iScrollBar);

	if(pScrollbarControl)
	{
		int iItemsPerPage = m_iRows * m_iColumns;

		CGUIMessage msg(GUI_MSG_LABEL_RESET, GetID(), pScrollbarControl->GetID(), iItemsPerPage, m_vecItems.size());
		g_windowManager.SendMessage(msg);
	}
}

void CGUIThumbnailPanel::OnPageUp()
{
	if(m_iPage > 1)
	{
		m_iPage--;
		UpdateCounterLabel();
		m_iOffset = (m_iPage-1)* m_iColumns * m_iRows;
	}
}

void CGUIThumbnailPanel::OnPageDown()
{
	int iItemsPerPage=m_iRows*m_iColumns;
	int iPages=m_vecItems.size() / iItemsPerPage;
	if(m_vecItems.size() % iItemsPerPage) iPages++;

	if(m_iPage + 1 <= iPages)
	{
		m_iPage++;
		UpdateCounterLabel();
		m_iOffset = (m_iPage-1) * iItemsPerPage;
	}

	while(m_iCursorX > 0 && m_iOffset + m_iCursorY * m_iColumns+m_iCursorX >= (int) m_vecItems.size())
		m_iCursorX--;

	while(m_iCursorY > 0 && m_iOffset + m_iCursorY * m_iColumns+m_iCursorX >= (int) m_vecItems.size())
		m_iCursorY--;
}

void CGUIThumbnailPanel::OnRight()
{
	if(m_iCursorX + 1 < m_iColumns && ValidItem(m_iCursorX + 1, m_iCursorY))
	{
		m_iCursorX++;
		return;
	}

	CGUIControl::OnRight();
 }

void CGUIThumbnailPanel::OnLeft()
{
	if(m_iCursorX > 0) 
	{
		m_iCursorX--;
		return;
	}

	CGUIControl::OnLeft();
}

void CGUIThumbnailPanel::OnUp()
{
	if(m_bScrollUp)
	{
		m_iScrollCounter = 0;
		m_bScrollUp = false;
		m_iOffset -= m_iColumns;
		m_iPage = m_iOffset / (m_iRows * m_iColumns);

		m_iPage += 1;

		UpdateCounterLabel();

//		CGUIMessage msg(GUI_MSG_ITEM_SELECT, 0, m_iScrollBar, m_iOffset);
//		g_windowManager.SendMessage(msg);
	}
	if(m_iCursorY > 0) 
	{
		m_iCursorY--; 
	}
	else if(m_iCursorY == 0 && m_iOffset)
	{
		m_iScrollCounter = m_iItemHeight;
		m_bScrollUp = true;
	}
	else
	{
//		return CGUIControl::OnUp();
	}
}

void CGUIThumbnailPanel::OnDown()
{
	if(m_bScrollDown)
	{
		m_bScrollDown = false;
		m_iOffset += m_iColumns;
		m_iPage = m_iOffset / (m_iRows * m_iColumns);

		m_iPage += 1;

		UpdateCounterLabel();

//		CGUIMessage msg(GUI_MSG_ITEM_SELECT, 0, m_iScrollBar, m_iOffset);
//		g_windowManager.SendMessage(msg);
	}
	if(m_iCursorY+1==m_iRows)
	{
		m_iOffset += m_iColumns;

		if(!ValidItem(m_iCursorX, m_iCursorY)) 
		{
			m_iOffset -= m_iColumns;
		}
		else
		{
			m_iOffset -= m_iColumns;
			m_iScrollCounter = m_iItemHeight;
			m_bScrollDown = true;
		}
		return;
	}
	else
	{
		if(ValidItem(m_iCursorX, m_iCursorY+1))
			m_iCursorY++;
	}
}

void CGUIThumbnailPanel::FreeResources()
{
	CGUIControl::FreeResources();

	m_imgFocus.FreeResources();
	m_imgNoFocus.FreeResources();
}

bool CGUIThumbnailPanel::OnAction(const CAction &action)
{
	switch (action.GetID())
	{
		case ACTION_SELECT_ITEM:
		{
			CGUIMessage msg(GUI_MSG_CLICKED, GetID(), GetParentID(), action.GetID());
			return g_windowManager.SendMessage(msg);
		}
		break;
	}

	CGUIControl::OnAction(action);

	if(action.GetID())
	{ 
		// Don't know what to do, so send to our parent window.
		CGUIMessage msg(GUI_MSG_CLICKED, GetID(), GetParentID(), action.GetID());
		return g_windowManager.SendMessage(msg);
	}

	return false;
}
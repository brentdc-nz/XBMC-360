#include "GUIListControl.h"
#include "GUIWindowManager.h"
#include "GUIScrollBarControl.h"
#include "GUILabelControl.h"

CGUIListControl::CGUIListControl(int parentID, int controlID, float fPosX, float fPosY, float fWidth, float fHeight,
                                 float fSpinWidth, float fSpinHeight,
 //                                const CStdString& strUp, const CStdString& strDown,
 //                                const CStdString& strUpFocus, const CStdString& strDownFocus,
 //                                const CLabelInfo & spinInfo, int iSpinX, int iSpinY,
                                 const CLabelInfo& labelInfo, //const CLabelInfo& labelInfo2,
                                 const CTextureInfo& TextureButton, const CTextureInfo& TextureButtonFocus)
	: CGUIControl(parentID, controlID, fPosX, fPosY, fWidth, fHeight)
	, m_imgButton(controlID, 0, fPosX, fPosY, fWidth, fHeight, TextureButtonFocus, TextureButton, labelInfo)
{
	m_label = labelInfo;
/*  m_label2 = labelInfo2;
  m_upDown.SetSpinAlign(XBFONT_CENTER_Y | XBFONT_RIGHT, 0);*/
	m_iOffset = 0;/*
  m_fSmoothScrollOffset = 0;*/
	m_iItemsPerPage = 10;
	m_iItemHeight = 45;/*
  m_iSelect = CONTROL_LIST;*/
	m_iCursorY = 0;/*
  m_strSuffix = "|";
  m_iSpinPosX = iSpinX;
  m_iSpinPosY = iSpinY;*/
	m_iImageWidth = 32;
	m_iImageHeight = 32;
	m_iSpaceBetweenItems = 10;/*
  m_bUpDownVisible = true;   // show the spin control by default
  m_upDown.SetShowRange(true); // show the range by default*/
	ControlType = GUICONTROL_LIST;
	m_iScrollBar = 0;
	m_iObjectCounterLabel = -1;
	m_iPage = 0;
}

CGUIListControl::~CGUIListControl(void)
{
}

void CGUIListControl::AllocResources()
{
	if(!m_label.font) return;
	CGUIControl::AllocResources();
	
	m_imgButton.AllocResources();

	SetHeight(m_height);
}

void CGUIListControl::FreeResources()
{
	CGUIControl::FreeResources();
	m_imgButton.FreeResources();
}

bool CGUIListControl::OnMessage(CGUIMessage& message)
{
	if(message.GetControlId() == GetID())
	{
		if(message.GetMessage() == GUI_MSG_ITEM_ADD)
		{
			CGUIListItem* pItem = (CGUIListItem*)message.GetLPVOID();
			m_vecItems.push_back(pItem);
			int iPages = m_vecItems.size() / m_iItemsPerPage;
			if(m_vecItems.size() % m_iItemsPerPage) iPages++;
//			m_upDown.SetRange(1, iPages);
//			m_upDown.SetValue(1);
			m_iPage = 1;
			UpdateCounterLabel();
			return true;
		}

		if(message.GetMessage() == GUI_MSG_LABEL_RESET)
		{
			m_iCursorY = 0;
			// Don't reset our page offset
//			m_iOffset = 0;
			m_vecItems.erase(m_vecItems.begin(), m_vecItems.end());
//			m_upDown.SetRange(1, 1);
//			m_upDown.SetValue(1);
			m_iPage = 1;
			UpdateCounterLabel();
			return true;
		}

		if(message.GetMessage() == GUI_MSG_ITEM_SELECTED)
		{
			message.SetParam1(m_iCursorY + m_iOffset);
			return true;
		}

		if(message.GetMessage() == GUI_MSG_SCROLL_CHANGE)
		{
			if(message.GetParam1() == GUI_MSG_SCROLL_UP)
				OnPageUp();
			else
				OnPageDown();

			return true;
		}

		if(message.GetMessage() == GUI_MSG_ITEM_SELECT)
		{
			// Check that m_iOffset is valid
			if(m_iOffset > (int)m_vecItems.size() - m_iItemsPerPage) m_iOffset = m_vecItems.size() - m_iItemsPerPage;
			if(m_iOffset < 0) m_iOffset = 0;
			
			// Only select an item if it's in a valid range
			if (message.GetParam1() >= 0 && message.GetParam1() < (int)m_vecItems.size())
			{
				// Select the item requested
				int iItem = message.GetParam1();
				
				if (iItem >= m_iOffset && iItem < m_iOffset + m_iItemsPerPage)
				{
					// The item is on the current page, so don't change it.
					m_iCursorY = iItem - m_iOffset;
				}
				else if (iItem < m_iOffset)
				{
					// Item is on a previous page - make it the first item on the page
					m_iCursorY = 0;
					m_iOffset = iItem;
				}
				else //(iItem >= m_iOffset+m_iItemsPerPage)
				{
					// Item is on a later page - make it the last item on the page
					m_iCursorY = m_iItemsPerPage - 1;
					m_iOffset = iItem - m_iCursorY;
				}
			}
			return true;
		}
	}

	if(CGUIControl::OnMessage(message)) return true;

	return false;
}

void CGUIListControl::Render()
{
	if(!IsVisible()) return;

	// First thing is we check the range of m_iOffset
	if (m_iOffset > (int)m_vecItems.size() - m_iItemsPerPage) m_iOffset = m_vecItems.size() - m_iItemsPerPage;
	if (m_iOffset < 0) m_iOffset = 0;

	// Loop through the list 3 times
	// 1. Render buttons & icons
	// 2. Render all text for font m_pFont
	// 3. Render all text for font m_pFont2
	// This is slightly faster than looping once through the list, render item, render m_pFont, render m_pFont2, render item, render m_pFont... etc
	// Text-rendering is generally slow and batching between Begin() End() makes it a bit faster. (XPR fonts)
	// Render buttons and icons

	float iPosY = m_posY;
	for(int i = 0; i < m_iItemsPerPage; i++)
	{
		float iPosX = m_posX;

		if(i + m_iOffset < (int)m_vecItems.size())
		{
			CGUIListItem *pItem = m_vecItems[i + m_iOffset];

			// Focused line
			m_imgButton.SetFocus(i == m_iCursorY && HasFocus()/* && m_iSelect == CONTROL_LIST*/);
			m_imgButton.SetPosition(m_posX, iPosY);
			m_imgButton.Render();

			// Render the icon
			if(pItem->HasThumbnail())
			{
				CStdString strThumb = pItem->GetThumbnailImage();
				
				//if (strThumb.Right(4) == ".tbn" || strThumb.Right(10) == "folder.jpg"))
				{
					// Copy the thumb -> icon
					pItem->SetIconImage(strThumb);
				}
			}
			
			if(pItem->HasIcon())
			{
				CStdString image = pItem->GetIconImage();
				bool bigImage(m_iImageWidth * m_iImageHeight > 1024); // bigger than 32x32
				
				if(bigImage && !pItem->HasThumbnail())
					image.Insert(image.Find("."), "Big");

			// Show icon
			CGUIImage* pImage = pItem->GetIcon();
			if(!pImage)
			{
				CTextureInfo TInfo;
				TInfo.filename = image;
				TInfo.useLarge = true;
				TInfo.orientation = 0;
				
				pImage = new CGUIImage(0, 0, 0, 0, m_iImageWidth, m_iImageHeight, TInfo/*, 0x0*/);
//				pImage->SetAspectRatio(CGUIImage::ASPECT_RATIO_KEEP);
				pItem->SetIcon(pImage);
			}

			if(pImage)
			{
				// Setting the filename will update the image if the thumb changes
				pImage->SetFileName(image);

				if(!pImage->IsAllocated())
					pImage->AllocResources();

				pImage->SetWidth(m_iImageWidth);
				pImage->SetHeight(m_iImageHeight);

				// Center vertically
				pImage->SetPosition(iPosX + 8 + (m_iImageWidth - /*pImage->GetRenderWidth()*/pImage->GetWidth()) / 2, iPosY + (m_iItemHeight - /*pImage->GetRenderHeight()*/pImage->GetHeight()) / 2);
				pImage->Render();

				if(bigImage)
				{
					// Add the overlay image if we're a big list
/*					CGUIImage *overlay = pItem->GetOverlay();

					if(!overlay && pItem->HasOverlay())
					{
						overlay = new CGUIImage(0, 0, 0, 0, 0, 0, pItem->GetOverlayImage(), 0x0);
						overlay->SetAspectRatio(CGUIImage::ASPECT_RATIO_KEEP);
						overlay->AllocResources();
						pItem->SetOverlay(overlay);
					}
					
					// Render the image
					if(overlay)
					{
						float x, y;
						pImage->GetBottomRight(x, y);
						
						// FIXME: fixed scaling to try and get it a similar size on MOST skins as
						//        small thumbs view
            
						float scale = 0.75f;
						overlay->SetWidth((int)(overlay->GetTextureWidth() * scale));
						overlay->SetHeight((int)(overlay->GetTextureHeight() * scale));
						
						// If we haven't yet rendered, make sure we update our sizing
						if(!overlay->HasRendered())
							overlay->CalculateSize();

						overlay->SetPosition((int)x - overlay->GetRenderWidth(), (int)y - overlay->GetRenderHeight());
						overlay->Render();
					}
	*/			}
			}
		}
			iPosY += m_iItemHeight + m_iSpaceBetweenItems;
		}
	}

	if(m_label.font)
	{
		// Calculate all our positions and sizes
		vector<CListText> labels;
		vector<CListText> labels2;
		iPosY = m_posY;
		for(int i = 0; i < m_iItemsPerPage; i++)
		{
			if(i + m_iOffset < (int)m_vecItems.size() )
			{
				CListText label; CListText label2;
				CGUIListItem *item = m_vecItems[i + m_iOffset];

				label.text = item->GetLabel();
//				label2.text = item->GetLabel2();
//				g_charsetConverter.utf8ToUTF16(item->GetLabel(), label.text);
//				g_charsetConverter.utf8ToUTF16(item->GetLabel2(), label2.text);

				// Calculate the position and size of our left label
				label.x = (float)m_posX + m_iImageWidth + m_label.offsetX + 10;
				label.y = (float)iPosY + m_label.offsetY;
				m_label.font->GetTextExtent(label.text.c_str(), &label.width, &label.height);

//				if(m_label.align & XBFONT_CENTER_Y)
//					label.y = (float)iPosY + (m_iItemHeight - label.height) * 0.5f;

				label.selected = item->IsSelected();
//				label.highlighted = (i == m_iCursorY && HasFocus()/* && m_iSelect == CONTROL_LIST*/);

			// Calculate the position and size of our right label
/*			if(!m_label2.offsetX)
				label2.x = (float)m_iPosX + m_dwWidth - 16;
			else
				label2.x = (float)m_iPosX + m_label2.offsetX;

			label2.y = (float)iPosY + m_label2.offsetY;

			if(label2.text.size() > 0 && m_label2.font)
			{
				m_label2.font->GetTextExtent(label2.text.c_str(), &label2.width, &label2.height);
				if(m_label.align & XBFONT_CENTER_Y)
				label2.y = (float)iPosY + (m_iItemHeight - label2.height) * 0.5f;
			}
			
			label2.selected = item->IsSelected();
			label2.highlighted = (i == m_iCursorY && HasFocus() && m_iSelect == CONTROL_LIST);
			label.maxwidth = (float)m_dwWidth - m_iImageWidth - m_label.offsetX - 20;
			label2.maxwidth = (float)label2.x - m_iPosX - m_iImageWidth - 20;
*/
			// Check whether they are going to overlap or not
			if(label.x + label.width > label2.x - label2.width)
			{
				// Overlap horizontally
				if((label.y <= label2.y && label2.y <= label.y + label.height) ||
					(label2.y <= label.y && label.y <= label2.y + label2.height))
			{ 
				// Overlap vertically
				// The labels overlap - we specify that:
				// 1.  The right label should take no more than half way
				// 2.  Both labels should be truncated if necessary
				float totalWidth = label2.x - label.x;
				float maxLabel2Width = max(totalWidth * 0.5f - 10, totalWidth - label.width - 5);

				if(label2.width > maxLabel2Width)
					label2.maxwidth = maxLabel2Width;
			
				label.maxwidth = totalWidth - min(label2.width, label2.maxwidth) - 5;
			}
		}
		// Move label2 from being right aligned to normal alignment for rendering
		label2.x -= min(label2.width, label2.maxwidth);
		labels.push_back(label);
		labels2.push_back(label2);
		iPosY += m_iItemHeight + m_iSpaceBetweenItems;
		}
	}

	//--------------------------------------------------------
	
	// Batch together all textrendering for m_label.font
//	m_label.font->Begin();
	for(vector<CListText>::iterator it = labels.begin(); it != labels.end(); it++)
	{
		RenderText(*it, m_label/*, m_scrollInfo*/);
	}
//	m_label.font->End();
	
	//--------------------------------------------------------
/*	
	// Batch together all textrendering for m_label2.font
	if(m_label2.font)
	{
		m_label2.font->Begin();
		for(vector<CListText>::iterator it = labels2.begin(); it != labels2.end(); it++)
		{
			RenderText(*it, m_label2, m_scrollInfo2);
		}
		m_label2.font->End();
		}
*/	}

	CGUIScrollBar* pScrollbarControl = NULL;
	pScrollbarControl = (CGUIScrollBar*)g_windowManager.GetWindow(g_windowManager.GetActiveWindow())->GetControl(m_iScrollBar);

	// FIXME: Don't send each frame
	if(pScrollbarControl)
	{
		CGUIMessage msg(GUI_MSG_LABEL_RESET, GetID(), pScrollbarControl->GetID(), m_iItemsPerPage, m_vecItems.size());
		g_windowManager.SendMessage(msg);
	}

	// FIXME: We should not be sending this each frame!
	CGUIMessage msg(GUI_MSG_ITEM_SELECT, 0, m_iScrollBar, m_iOffset);
	g_windowManager.SendMessage(msg);

 	CGUIControl::Render();
}

bool CGUIListControl::OnAction(const CAction &action) // TODO: Scrolling with triggers
{
	switch(action.GetID())
	{
/*		// Smooth scrolling (for analog controls)
		case ACTION_SCROLL_UP:
		{
			m_fSmoothScrollOffset += action.fAmount1 * action.fAmount1;
			bool handled = false;
			while(m_fSmoothScrollOffset > 0.4)
			{
				handled = true;
				m_fSmoothScrollOffset -= 0.4f;
				if(m_iOffset > 0 && m_iCursorY <= m_iItemsPerPage / 2)
				{
					Scroll( -1);
				}
				else if (m_iCursorY > 0)
				{
					m_iCursorY--;
				}
			}
			return handled;
		}
		break;

		case ACTION_SCROLL_DOWN:
		{
			m_fSmoothScrollOffset += action.fAmount1 * action.fAmount1;
			bool handled = false;
			while(m_fSmoothScrollOffset > 0.4)
			{
				handled = true;
				m_fSmoothScrollOffset -= 0.4f;
				if(m_iOffset + m_iItemsPerPage < (int)m_vecItems.size() && m_iCursorY >= m_iItemsPerPage / 2)
				{
					Scroll(1);
				}
				else if(m_iCursorY < m_iItemsPerPage - 1 && m_iOffset + m_iCursorY < (int)m_vecItems.size() - 1)
				{
					m_iCursorY++;
				}
			}
			return handled;
		}
		break;
*/
		case ACTION_MOVE_LEFT:
		case ACTION_MOVE_RIGHT:
		case ACTION_MOVE_DOWN:
		case ACTION_MOVE_UP:
		{ 
			// Use base class implementation
			return CGUIControl::OnAction(action);
		}
		break;

		default:
		{
			if(/*m_iSelect == CONTROL_LIST &&*/ action.GetID())
			{ 
				// Don't know what to do, so send to our parent window.
				CGUIMessage msg(GUI_MSG_CLICKED, GetID(), GetParentID(), action.GetID());
				return g_windowManager.SendMessage(msg);
			}
			else
			{
//				// Send action to the page control
//				return m_upDown.OnAction(action);
				return false;
			}
		}
	}
}

void CGUIListControl::RenderText(const CListText &text, const CLabelInfo &label/*, CScrollInfo &scroll*/) // FIXME: Text colour stuff
{
	if(!label.font)
		return;

	static int iLastItem = -1;

	DWORD color = text.selected ? label.dwSelectedTextColor : label.dwTextColor;
	
	if(!text.highlighted)
	{
		label.font->DrawTextWidth(text.x, text.y, color/*, label.shadowColor*/, text.text, /*text.maxwidth*/500); //FIXME
		return;
	}
	else
	{
		if(text.width <= text.maxwidth)
		{
			// Don't need to scroll
			label.font->DrawTextWidth(text.x, text.y, color/*, label.shadowColor*/, text.text, text.maxwidth);
//			scroll.Reset();
			return;
		}
   
		// Scroll
		CStdStringW scrollString(text.text);
		scrollString += L" ";
//		scrollString += m_strSuffix;
		int iItem = m_iCursorY + m_iOffset;
//		label.font->End(); // need to deinit the font before setting viewport
		
		if(iLastItem != iItem)
		{
//			scroll.Reset();
			iLastItem = iItem;
		}
//		label.font->DrawScrollingText(text.x, text.y, &color, 1, label.shadowColor, scrollString, text.maxwidth, scroll);
//		label.font->Begin(); // resume fontbatching
	}
}

void CGUIListControl::OnUp()
{
	if(/*m_iSelect == CONTROL_LIST*/1)
	{
		if(m_iCursorY > 0)
		{
			m_iCursorY--;
		}
		else if(m_iCursorY == 0 && m_iOffset)
		{
			m_iOffset--;
		}
		else if(m_dwControlUp == 0 || m_dwControlUp == GetID())
		{
			if(m_vecItems.size() > 0)
			{
				// move 2 last item in list
				m_iOffset = m_vecItems.size() - m_iItemsPerPage;
				if(m_iOffset < 0) m_iOffset = 0;
				m_iCursorY = m_vecItems.size() - m_iOffset - 1;
			}
		}
		else
		{
			CGUIControl::OnUp();
		}
	}
	else
	{
		// Focus the list again
//		m_upDown.SetFocus(false);
//		m_iSelect = CONTROL_LIST;
	}
}

void CGUIListControl::OnDown()
{
	if(/*m_iSelect == CONTROL_LIST*/1)
	{
		if(m_iOffset + 1 + m_iCursorY < (int)m_vecItems.size())
		{
			if(m_iCursorY + 1 < m_iItemsPerPage)
				m_iCursorY++;
			else
				m_iOffset++;
		}
		else if(m_dwControlDown == 0 || m_dwControlDown == GetID())
		{
			// Move first item in list
			m_iOffset = 0;
			m_iCursorY = 0;
		}
		else
		{
			CGUIControl::OnDown();
		}
	}
	else
	{
		// Move down off our control
//		m_upDown.SetFocus(false);
		CGUIControl::OnDown();
	}
}

void CGUIListControl::OnPageUp()
{
	int iPage = m_iPage;

	if(iPage > 1)
	{
		iPage--;
		m_iPage = iPage;
		m_iOffset = (m_iPage - 1) * m_iItemsPerPage;
	}
	else
	{
		// Already on page 1, then select the 1st item
		m_iCursorY = 0;
	}

	UpdateCounterLabel();
}

void CGUIListControl::OnPageDown()
{
	int iPages = m_vecItems.size() / m_iItemsPerPage;

	if(m_vecItems.size() % m_iItemsPerPage)
		iPages++;

	int iPage = m_iPage;

	if(iPage + 1 <= iPages)
	{
		iPage++;
		m_iPage = iPage;
		m_iOffset = (m_iPage - 1) * m_iItemsPerPage;
	}
	else
	{
		// Already on last page, move 2 last item in list
		CGUIMessage msg(GUI_MSG_ITEM_SELECT, GetID(), GetID(), m_vecItems.size() - 1);
		OnMessage(msg);
	}

	if(m_iOffset + m_iCursorY >= (int)m_vecItems.size() )
	{
		m_iCursorY = (m_vecItems.size() - m_iOffset) - 1;
	}

	UpdateCounterLabel();
}

void CGUIListControl::SetSpace(int iHeight)
{
	m_iSpaceBetweenItems = iHeight;
}

void CGUIListControl::SetScrollbarControl(const int iControl)
{
	m_iScrollBar = iControl;
}

void CGUIListControl::SetObjectLabelControl(const int iControl)
{
	m_iObjectCounterLabel = iControl;
}

void CGUIListControl::SetHeight(float iHeight)
{
//	int iSpinOffsetY = m_upDown.GetYPosition() - GetYPosition() - GetHeight();
	CGUIControl::SetHeight(iHeight);
	m_imgButton.SetHeight(m_iItemHeight);
//	m_upDown.SetPosition(m_upDown.GetXPosition(), GetYPosition() + GetHeight() + iSpinOffsetY);

	float fHeight = (float)m_iItemHeight + (float)m_iSpaceBetweenItems;
	float fTotalHeight = (float)(m_height/* - m_upDown.GetHeight()*/ - 5);
	m_iItemsPerPage = (int)(fTotalHeight / fHeight);

	int iPages = m_vecItems.size() / m_iItemsPerPage;
	if(m_vecItems.size() % m_iItemsPerPage) iPages++;
//	m_upDown.SetRange(1, iPages);
}

void CGUIListControl::UpdateCounterLabel()
{
	// Update our counter label
	const CGUILabelControl* pLabelControl = NULL;
	pLabelControl = (CGUILabelControl*)g_windowManager.GetWindow(g_windowManager.GetActiveWindow())->GetControl(m_iObjectCounterLabel);
	
	if(pLabelControl && (m_iObjectCounterLabel != -1))
	{
		CStdString strTemp;
		strTemp.Format("%i Objects - Pages %i / %i", m_vecItems.size(), m_iPage, m_vecItems.size() / m_iItemsPerPage + 1);

		CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), pLabelControl->GetID());
		msg.SetLabel(strTemp);
		g_windowManager.SendMessage(msg);
	}
}
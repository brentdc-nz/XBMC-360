#include "include.h"
#include "GUIControlGroupList.h"
#include "Key.h"
#include "GUIInfoManager.h"
#include "GUIFont.h" // For XBFONT_* definitions

CGUIControlGroupList::CGUIControlGroupList(int parentID, int controlID, float posX, float posY, float width, float height, float itemGap, int pageControl, ORIENTATION orientation, bool useControlPositions, uint32_t alignment, unsigned int scrollTime)
: CGUIControlGroup(parentID, controlID, posX, posY, width, height)
{
	m_itemGap = itemGap;
	m_pageControl = pageControl;
	m_offset = 0;
	m_totalSize = 10;
	m_orientation = orientation;
	m_alignment = alignment;
	m_scrollOffset = 0;
	m_scrollSpeed = 0;
	m_scrollLastTime = 0;
	m_scrollTime = scrollTime ? scrollTime : 1;
	m_renderTime = 0;
	m_useControlPositions = useControlPositions;
	ControlType = GUICONTROL_GROUPLIST;
}

CGUIControlGroupList::~CGUIControlGroupList(void)
{
}

void CGUIControlGroupList::Render()
{
	if (m_scrollSpeed != 0)
	{
		m_offset += m_scrollSpeed * (m_renderTime - m_scrollLastTime);
		if ((m_scrollSpeed < 0 && m_offset < m_scrollOffset) ||
			(m_scrollSpeed > 0 && m_offset > m_scrollOffset))
		{
			m_offset = m_scrollOffset;
			m_scrollSpeed = 0;
		}
	}
	m_scrollLastTime = m_renderTime;

	// First we update visibility of all our items, to ensure our size and
	// alignment computations are correct.
	for (iControls it = m_children.begin(); it != m_children.end(); ++it)
		(*it)->UpdateVisibility();

	ValidateOffset();

	if (m_pageControl)
	{
		CGUIMessage message(GUI_MSG_LABEL_RESET, GetParentID(), m_pageControl, (int)m_height, (int)m_totalSize);
		SendWindowMessage(message);

		CGUIMessage message2(GUI_MSG_ITEM_SELECT, GetParentID(), m_pageControl, (int)m_offset);
		SendWindowMessage(message2);
	}

	// We run through the controls, rendering as we go
	bool render(g_graphicsContext.SetClipRegion(m_posX, m_posY, m_width, m_height));
	float pos = GetAlignOffset();
	float focusedPos = 0;
	CGUIControl *focusedControl = NULL;

	for (iControls it = m_children.begin(); it != m_children.end(); ++it)
	{
		// Note we render all controls, even if they're offscreen, as then they'll be updated
		// with respect to animations
		CGUIControl *control = *it;
		if (m_renderFocusedLast && control->HasFocus())
		{
			focusedControl = control;
			focusedPos = pos;
		}
		else
		{
			if (m_orientation == VERTICAL)
				g_graphicsContext.SetOrigin(m_posX, m_posY + pos - m_offset);
			else
				g_graphicsContext.SetOrigin(m_posX + pos - m_offset, m_posY);
			
			control->DoRender(m_renderTime);
		}

		if (control->IsVisible())
			pos += Size(control) + m_itemGap;

		g_graphicsContext.RestoreOrigin();
	}

	if (focusedControl)
	{
		if (m_orientation == VERTICAL)
			g_graphicsContext.SetOrigin(m_posX, m_posY + focusedPos - m_offset);
		else
			g_graphicsContext.SetOrigin(m_posX + focusedPos - m_offset, m_posY);

		focusedControl->DoRender(m_renderTime);
	}

	if (render) g_graphicsContext.RestoreClipRegion();
		CGUIControl::Render();
}

bool CGUIControlGroupList::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage() )
	{
		case GUI_MSG_FOCUSED:
		{
			// A control has been focused
			// scroll if we need to and update our page control
			ValidateOffset();
			float offset = 0;
			
			for (iControls it = m_children.begin(); it != m_children.end(); ++it)
			{
				CGUIControl *control = *it;
				
				if (!control->IsVisible())
					continue;
				
				if (control->HasID(message.GetControlId()))
				{
					// Find out whether this is the first or last control
					if (IsFirstFocusableControl(control))
						ScrollTo(0);
					else if (IsLastFocusableControl(control))
						ScrollTo(m_totalSize - Size());
					else if (offset < m_offset)
						ScrollTo(offset);
					else if (offset + Size(control) > m_offset + Size())
						ScrollTo(offset + Size(control) - Size());
					break;
				}
				offset += Size(control) + m_itemGap;
			}
		}
		break;
		case GUI_MSG_SETFOCUS:
		{
			// We've been asked to focus.  We focus the last control if it's on this page,
			// else we'll focus the first focusable control from our offset (after verifying it)
			ValidateOffset();
			
			// Now check the focusControl's offset
			float offset = 0;
			for (iControls it = m_children.begin(); it != m_children.end(); ++it)
			{
				CGUIControl *control = *it;

				if (!control->IsVisible())
					continue;
			
				if (control->HasID(m_focusedControl))
				{
					if (offset >= m_offset && offset + Size(control) <= m_offset + Size())
						return CGUIControlGroup::OnMessage(message);

					break;
				}
				offset += Size(control) + m_itemGap;
			}
			
			// Find the first control on this page
			offset = 0;
			for (iControls it = m_children.begin(); it != m_children.end(); ++it)
			{
				CGUIControl *control = *it;

				if (!control->IsVisible())
					continue;
				
				if (control->CanFocus() && offset >= m_offset && offset + Size(control) <= m_offset + Size())
				{
					m_focusedControl = control->GetID();
					break;
				}
				offset += Size(control) + m_itemGap;
			}
		}
		break;
		case GUI_MSG_PAGE_CHANGE:
		{
			if (message.GetSenderId() == m_pageControl)
			{
				// It's from our page control
				ScrollTo((float)message.GetParam1());
				return true;
			}
		}
		break;
	}
	return CGUIControlGroup::OnMessage(message);
}

void CGUIControlGroupList::ValidateOffset()
{
	// Calculate how many items we have on this page
	m_totalSize = 0;

	for (iControls it = m_children.begin(); it != m_children.end(); ++it)
	{
		CGUIControl *control = *it;
		if (!control->IsVisible()) continue;
		m_totalSize += Size(control) + m_itemGap;
	}

	if (m_totalSize > 0) m_totalSize -= m_itemGap;
	
	// Check our m_offset range
	if (m_offset > m_totalSize - Size())
		m_offset = m_totalSize - Size();
	
	if (m_offset < 0) m_offset = 0;
}

void CGUIControlGroupList::AddControl(CGUIControl *control, int position /*= -1*/)	
{
	// NOTE: We override control navigation here, but we don't override the <onleft> etc. builtins
	//       if specified.
	if (position < 0 || position > (int)m_children.size()) // add at the end
		position = (int)m_children.size();

	if (control)
	{
		// Set the navigation of items so that they form a list
		int beforeID = (m_orientation == VERTICAL) ? GetControlIdUp() : GetControlIdLeft();
		int afterID = (m_orientation == VERTICAL) ? GetControlIdDown() : GetControlIdRight();
		
		if (m_children.size())
		{
			// We're inserting at the given position, so grab the items above and below and alter
			// their navigation accordingly
			CGUIControl *before = NULL;
			CGUIControl *after = NULL;
			
			if (position == 0)
			{
				// Inserting at the beginning
				after = m_children[0];

				if (afterID == GetID()) // We're wrapping around bottom->top, so we have to update the last item
					before = m_children[m_children.size() - 1];
				
				if (beforeID == GetID()) // We're wrapping around top->bottom
					beforeID = m_children[m_children.size() - 1]->GetID();
				
				afterID = after->GetID();
			}
			else if (position == (int)m_children.size())
			{
				// Inserting at the end
				before = m_children[m_children.size() - 1];
				
				if (beforeID == GetID()) // We're wrapping around top->bottom, so we have to update the first item
					after = m_children[0];
				
				if (afterID == GetID()) // We're wrapping around bottom->top
					afterID = m_children[0]->GetID();
				
				beforeID = before->GetID();
			}
			else
			{
				// Inserting somewhere in the middle
				before = m_children[position - 1];
				after = m_children[position];
				beforeID = before->GetID();
				afterID = after->GetID();
			}
			if (m_orientation == VERTICAL)
			{
				if (before) // Update the DOWN action to point to us
					before->SetNavigation(before->GetControlIdUp(), control->GetID(), GetControlIdLeft(), GetControlIdRight(), GetControlIdBack());

				if (after) // Update the UP action to point to us
					after->SetNavigation(control->GetID(), after->GetControlIdDown(), GetControlIdLeft(), GetControlIdRight(), GetControlIdBack());
			}
			else
			{
				if (before) // Update the RIGHT action to point to us
					before->SetNavigation(GetControlIdUp(), GetControlIdDown(), before->GetControlIdLeft(), control->GetID(), GetControlIdBack());

				if (after) // Update the LEFT action to point to us
					after->SetNavigation(GetControlIdUp(), GetControlIdDown(), control->GetID(), after->GetControlIdRight(), GetControlIdBack());
			}
		}
		
		// Now the control's nav
		CGUIAction empty;
		if (m_orientation == VERTICAL)
		{
			control->SetNavigation(beforeID, afterID, GetControlIdLeft(), GetControlIdRight(), GetControlIdBack());
			control->SetNavigationActions(empty, empty, m_actionLeft, m_actionRight, empty, false);
		}
		else
		{
			control->SetNavigation(GetControlIdUp(), GetControlIdDown(), beforeID, afterID, GetControlIdBack());
			control->SetNavigationActions(m_actionUp, m_actionDown, empty, empty, empty, false);
		}

		if (!m_useControlPositions)
			control->SetPosition(0,0);
		
		CGUIControlGroup::AddControl(control, position);
	}
}

void CGUIControlGroupList::ClearAll()
{
	CGUIControlGroup::ClearAll();
	m_offset = 0;
}

inline float CGUIControlGroupList::Size(const CGUIControl *control) const
{
	return (m_orientation == VERTICAL) ? control->GetYPosition() + control->GetHeight() : control->GetXPosition() + control->GetWidth();
}

inline float CGUIControlGroupList::Size() const
{
	return (m_orientation == VERTICAL) ? m_height : m_width;
}

void CGUIControlGroupList::ScrollTo(float offset)
{
	m_scrollOffset = offset;
	m_scrollSpeed = (m_scrollOffset - m_offset) / m_scrollTime;
}

#ifdef _HAS_MOUSE
bool CGUIControlGroupList::SendMouseEvent(const CPoint &point, const CMouseEvent &event)
{
	// Transform our position into child coordinates
	CPoint childPoint(point);
	m_transform.InverseTransformPosition(childPoint.x, childPoint.y);
	
	if (CGUIControl::CanFocus())
	{
		float pos = 0;
		float alignOffset = GetAlignOffset();
		
		for (ciControls i = m_children.begin(); i != m_children.end(); ++i)
		{
			CGUIControl *child = *i;
			if (child->IsVisible())
			{
				if (pos + Size(child) > m_offset && pos < m_offset + Size())
				{
					// We're on screen
					float offsetX = m_orientation == VERTICAL ? m_posX : m_posX + alignOffset + pos - m_offset;
					float offsetY = m_orientation == VERTICAL ? m_posY + alignOffset + pos - m_offset : m_posY;
					
					if (child->SendMouseEvent(childPoint - CPoint(offsetX, offsetY), event))
					{
						// We've handled the action, and/or have focused an item
						return true;
					}
				}
				pos += Size(child) + m_itemGap;
			}
		}
		// None of our children want the event, but we may want it
		if (HitTest(childPoint) && OnMouseEvent(childPoint, event))
			return true;
	}
	m_focusedControl = 0;
	return false;
}
#endif

void CGUIControlGroupList::UnfocusFromPoint(const CPoint &point)
{
	float pos = 0;
	CPoint controlCoords(point);
	m_transform.InverseTransformPosition(controlCoords.x, controlCoords.y);
	float alignOffset = GetAlignOffset();

	for (iControls it = m_children.begin(); it != m_children.end(); ++it)
	{
		CGUIControl *child = *it;
		if (child->IsVisible())
		{
			if (pos + Size(child) > m_offset && pos < m_offset + Size())
			{
				// We're on screen
				CPoint offset = (m_orientation == VERTICAL) ? CPoint(m_posX, m_posY + alignOffset + pos - m_offset) : CPoint(m_posX + alignOffset + pos - m_offset, m_posY);
				child->UnfocusFromPoint(controlCoords - offset);
			}
			pos += Size(child) + m_itemGap;
		}
	}
	CGUIControl::UnfocusFromPoint(point);
}

bool CGUIControlGroupList::GetCondition(int condition, int data) const
{
	switch (condition)
	{
		case CONTAINER_HAS_NEXT:
			return (m_totalSize >= Size() && m_offset < m_totalSize - Size());
		case CONTAINER_HAS_PREVIOUS:
			return (m_offset > 0);
		default:
			return false;
	}
}

bool CGUIControlGroupList::IsFirstFocusableControl(const CGUIControl *control) const
{
	for (ciControls it = m_children.begin(); it != m_children.end(); ++it)
	{
		CGUIControl *child = *it;
		if (child->IsVisible() && child->CanFocus())
		{
			// Found first focusable
			return child == control;
		}
	}
	return false;
}

bool CGUIControlGroupList::IsLastFocusableControl(const CGUIControl *control) const
{
	for (crControls it = m_children.rbegin(); it != m_children.rend(); ++it)
	{
		CGUIControl *child = *it;
		if (child->IsVisible() && child->CanFocus())
		{
			// Found first focusable
			return child == control;
		}
	}
	return false;
}

float CGUIControlGroupList::GetAlignOffset() const
{
	if (m_totalSize < Size())
	{
		if (m_alignment & XBFONT_RIGHT)
			return Size() - m_totalSize;

		if (m_alignment & XBFONT_CENTER_X)
			return (Size() - m_totalSize)*0.5f;
	}
	return 0.0f;
}

#ifdef _HAS_MOUSE
bool CGUIControlGroupList::OnMouseEvent(const CPoint &point, const CMouseEvent &event)
{
	if (event.m_id == ACTION_MOUSE_WHEEL_UP || event.m_id == ACTION_MOUSE_WHEEL_DOWN)
	{
		// Find the current control and move to the next or previous
		float offset = 0;
		for (ciControls it = m_children.begin(); it != m_children.end(); ++it)
		{
			CGUIControl *control = *it;
			if (!control->IsVisible()) continue;
			float nextOffset = offset + Size(control) + m_itemGap;
			
			if (event.m_id == ACTION_MOUSE_WHEEL_DOWN && nextOffset > m_offset) // Past our current offset
			{
				ScrollTo(nextOffset);
				return true;
			}
			else if (event.m_id == ACTION_MOUSE_WHEEL_UP && nextOffset >= m_offset) // At least at our current offset
			{
				ScrollTo(offset);
				return true;
			}
			offset = nextOffset;
		}
	}
	return false;
}
#endif

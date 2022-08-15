#include "GUIControlGroup.h"

CGUIControlGroup::CGUIControlGroup()
{
	m_defaultControl = 0;
	m_defaultAlways = false;
	m_focusedControl = 0;
	m_renderTime = 0;
	m_renderFocusedLast = false;
	ControlType = GUICONTROL_GROUP;
}

CGUIControlGroup::CGUIControlGroup(int parentID, int controlID, float posX, float posY, float width, float height)
: CGUIControl(parentID, controlID, posX, posY, width, height)
{
	m_defaultControl = 0;
	m_defaultAlways = false;
	m_focusedControl = 0;
	m_renderTime = 0;
	m_renderFocusedLast = false;
	ControlType = GUICONTROL_GROUP;
}

CGUIControlGroup::~CGUIControlGroup(void)
{
	ClearAll();
}

void CGUIControlGroup::AllocResources()
{
	CGUIControl::AllocResources();

	for(iControls it = m_children.begin(); it != m_children.end(); ++it)
	{
		CGUIControl *control = *it;
		if(!control->IsDynamicallyAllocated())
			control->AllocResources();
	}
}

void CGUIControlGroup::FreeResources(bool immediately)
{
	CGUIControl::FreeResources(/*immediately*/); //TODO

	for(iControls it = m_children.begin(); it != m_children.end(); ++it)
	{
		CGUIControl *control = *it;
		control->FreeResources(/*immediately*/); //TODO
	}
}

void CGUIControlGroup::Render()
{
	CPoint pos(GetPosition());
	g_graphicsContext.SetOrigin(pos.x, pos.y);

	CGUIControl *focusedControl = NULL;

	for(iControls it = m_children.begin(); it != m_children.end(); ++it)
	{
		CGUIControl *control = *it;
//		control->UpdateVisibility();

		if(m_renderFocusedLast && control->HasFocus())
			focusedControl = control;
		else
			control->DoRender(m_renderTime);
	}

	if(focusedControl)
		focusedControl->DoRender(m_renderTime);

	CGUIControl::Render();
	g_graphicsContext.RestoreOrigin();
}

void CGUIControlGroup::DoRender(unsigned int currentTime)
{
	m_renderTime = currentTime;
	CGUIControl::DoRender(currentTime);
}

void CGUIControlGroup::AddControl(CGUIControl *control, int position /* = -1*/)
{
	if(!control) return;

	if(position < 0 || position > (int)m_children.size())
		position = (int)m_children.size();

	m_children.insert(m_children.begin() + position, control);
	control->SetParentControl(this);
//	control->SetPushUpdates(m_pushedUpdates);
	AddLookup(control);
}

void CGUIControlGroup::AddLookup(CGUIControl *control)
{
	if(control->IsGroup())
	{
		// First add all the subitems of this group (if they exist)
		const LookupMap map = ((CGUIControlGroup *)control)->GetLookup();

		for(LookupMap::const_iterator i = map.begin(); i != map.end(); i++)
			m_lookup.insert(m_lookup.upper_bound(i->first), make_pair(i->first, i->second));
	}

	if(control->GetID())
		m_lookup.insert(m_lookup.upper_bound(control->GetID()), make_pair(control->GetID(), control));
	
	// Ensure that our size is what it should be
	if(m_parentControl)
		((CGUIControlGroup*)m_parentControl)->AddLookup(control);
}

void CGUIControlGroup::RemoveLookup(CGUIControl *control)
{
	if(control->IsGroup())
	{
		// Remove the group's lookup
		const LookupMap &map = ((CGUIControlGroup *)control)->GetLookup();
		for(LookupMap::const_iterator i = map.begin(); i != map.end(); i++)
		{
			// Remove this control
			for(LookupMap::iterator it = m_lookup.begin(); it != m_lookup.end(); it++)
			{
				if(i->second == it->second)
				{
					m_lookup.erase(it);
					break;
				}
			}
		}
	}
	
	// Remove the actual control
	if(control->GetID())
	{
		for(LookupMap::iterator it = m_lookup.begin(); it != m_lookup.end(); it++)
		{
			if(control == it->second)
			{
				m_lookup.erase(it);
				break;
			}
		}
	}

	if(m_parentControl)
		((CGUIControlGroup *)m_parentControl)->RemoveLookup(control);
}

void CGUIControlGroup::ClearAll()
{
	// First remove from the lookup table
	if(m_parentControl)
	{
		for(iControls it = m_children.begin(); it != m_children.end(); it++)
			((CGUIControlGroup*)m_parentControl)->RemoveLookup(*it);
	}

	// and delete all our children
	for(iControls it = m_children.begin(); it != m_children.end(); it++)
	{
		CGUIControl *control = *it;
		delete control;
	}

	m_children.clear();
	m_lookup.clear();
}
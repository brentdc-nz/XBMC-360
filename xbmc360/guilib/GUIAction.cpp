#include "GUIAction.h"
#include "utils/StringUtils.h"
#include "GUIWindowManager.h"
#include "GUIControl.h"
#include "GUIInfoManager.h"
#include "utils\StringUtils.h"

using namespace std;

CGUIAction::CGUIAction()
{
	m_sendThreadMessages = false;
}

bool CGUIAction::Execute(int controlID, int parentID, int direction /*= 0*/) const
{
	if (m_actions.size() == 0) return false;

	bool retval = false;
	CGUIAction copy(*this);

	for (ciActions it = copy.m_actions.begin() ; it != copy.m_actions.end() ; it++)
	{
		if (it->condition.IsEmpty() || g_infoManager.EvaluateBool(it->condition))
		{
			if(CStringUtils::IsInteger(it->action))
			{
				CGUIMessage msg(GUI_MSG_MOVE, parentID, controlID, direction);
				if (parentID)
				{
					CGUIWindow *pWindow = g_windowManager.GetWindow(parentID);
					if (pWindow)
					{
						retval |= pWindow->OnMessage(msg);
						continue;
					}
				}
				retval |= g_windowManager.SendMessage(msg);
			}
			else
			{
				CGUIMessage msg(GUI_MSG_EXECUTE, controlID, parentID);
				msg.SetStringParam(it->action);

				if (m_sendThreadMessages)
					g_windowManager.SendThreadMessage(msg);
				else
					g_windowManager.SendMessage(msg);

				retval |= true;
			}
		}
	}
	return retval;
}

int CGUIAction::GetNavigation() const
{
	for (ciActions it = m_actions.begin() ; it != m_actions.end() ; it++)
	{
		if (CStringUtils::IsInteger(it->action))
		{
			if (it->condition.IsEmpty() || g_infoManager.EvaluateBool(it->condition))
				return atoi(it->action.c_str());
		}
	}
	return 0;
}

void CGUIAction::SetNavigation(int id)
{
	if (id == 0) return;

	CStdString strId;
	strId.Format("%i", id);

	for (iActions it = m_actions.begin() ; it != m_actions.end() ; it++)
	{
		if (CStringUtils::IsInteger(it->action) && it->condition.IsEmpty())
		{
			it->action = strId;
			return;
		}
	}

	cond_action_pair pair;
	pair.action = strId;
	m_actions.push_back(pair);
}

bool CGUIAction::HasActionsMeetingCondition() const
{
	for (ciActions it = m_actions.begin() ; it != m_actions.end() ; it++)
	{
		if (it->condition.IsEmpty() || g_infoManager.EvaluateBool(it->action))
			return true;
	}
	return false;
}
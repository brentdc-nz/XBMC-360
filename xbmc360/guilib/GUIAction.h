#ifndef H_CGUIACTION
#define H_CGUIACTION

#include <vector>
#include "utils/StdString.h"

class CGUIControl;

/**
 * Class containing vector of condition->(action/navigation route) pairs and handling its execution.
 */
class CGUIAction
{
public:
	CGUIAction();

	// Execute actions, if action is paired with condition - Evaluate condition first
	bool Execute(int controlID, int parentID, int direction = 0) const;

	// Check if there is any action that meet its condition
	bool HasActionsMeetingCondition() const;

	// Check if there is any action
	bool HasAnyActions() const { return m_actions.size() > 0; };

	// Get navigation route that meet its conditions first
	int GetNavigation() const;

	// Set navigation route
	void SetNavigation(int id);

	// GetFirstAction is only needed in deprecated http api
	CStdString GetFirstAction() const { return m_actions.size() > 0 ? m_actions[0].action : ""; };

private:
	struct cond_action_pair
	{
		CStdString condition;
		CStdString action;
	};

	std::vector<cond_action_pair> m_actions;
	bool m_sendThreadMessages;

	typedef std::vector<cond_action_pair>::const_iterator ciActions;
	typedef std::vector<cond_action_pair>::iterator iActions;
	friend class CGUIControlFactory; // No need for setters / adders
};

#endif //H_CGUIACTION

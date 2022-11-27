#ifndef CINFOBOOL_H
#define CINFOBOOL_H

#include <vector>
#include <map>
#include "utils\StdString.h"

class CGUIListItem;

namespace INFO
{
// Base class, wrapping boolean conditions and expressions

class InfoBool
{
public:
	InfoBool(const CStdString &expression, int context)
		: m_value(false),

		m_context(context),
		m_expression(expression),
		m_lastUpdate(0)
	{
	};

	// Get the value of this info bool
	// This is called to update (if necessary) and fetch the value of the info bool
	// param time current time (used to test if we need to update yet)
	// param item the item used to evaluate the bool
	inline bool Get(unsigned int time, const CGUIListItem *item = NULL)
	{
		if (item)
			Update(item);
		else if (time - m_lastUpdate > 0)
		{
			Update(item);
			m_lastUpdate = time;
		}
		return m_value;
	}

	bool operator==(const InfoBool &right) const
	{
		return (m_context == right.m_context && 
			m_expression.CompareNoCase(right.m_expression) == 0);
	}

	// Update the value of this info bool
	// This is called if and only if the info bool is dirty, allowing it to update it's current value
	virtual void Update(const CGUIListItem *item) {};

protected:
	bool m_value;                // Current value
	int m_context;               // Contextual information to go with the condition

private:
	CStdString m_expression;     // Original expression
	unsigned int m_lastUpdate;   // Last update time (to determine dirty status)
};

// Class to wrap active boolean conditions
class InfoSingle : public InfoBool
{
public:
	InfoSingle(const CStdString &condition, int context);
	virtual void Update(const CGUIListItem *item);

private:
	int m_condition; // Actual condition this represents
};

// Class to wrap active boolean expressions
class InfoExpression : public InfoBool
{
public:
	InfoExpression(const CStdString &expression, int context);
	virtual void Update(const CGUIListItem *item);

private:
	void Parse(const CStdString &expression);
	bool Evaluate(const CGUIListItem *item, bool &result);
	short GetOperator(const char ch) const;

	std::vector<short> m_postfix;         // The postfix form of the expression (operators and operand indicies)
	std::vector<unsigned int> m_operands; // The operands in the expression
};

};

#endif //CINFOBOOL_H
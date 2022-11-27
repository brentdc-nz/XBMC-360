#ifndef CSKINVARIABLE_H
#define CSKINVARIABLE_H

#include "guilib\GUIInfoTypes.h"

class TiXmlElement;

namespace INFO
{
class CSkinVariableString;

class CSkinVariable
{
public:
	static const CSkinVariableString* CreateFromXML(const TiXmlElement& node, int context);
};

class CSkinVariableString
{
public:
	const CStdString& GetName() const;
	int GetContext() const;
	CStdString GetValue(bool preferImage = false, const CGUIListItem *item = NULL );

private:
	CSkinVariableString();

	CStdString m_name;
	int m_context;

	struct ConditionLabelPair
	{
		int m_condition;
		CGUIInfoLabel m_label;
	};

	typedef std::vector<ConditionLabelPair> VECCONDITIONLABELPAIR;
	VECCONDITIONLABELPAIR m_conditionLabelPairs;

	friend class CSkinVariable;
};

}

#endif //CSKINVARIABLESTRING_H
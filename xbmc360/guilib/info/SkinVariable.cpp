#include "SkinVariable.h"
#include "guilib\GUIInfoManager.h"
#include "guilib\tinyXML\tinyxml.h"

using namespace std;
using namespace INFO;

#define DEFAULT_VALUE -1

const CSkinVariableString* CSkinVariable::CreateFromXML(const TiXmlElement& node, int context)
{
	const char* name = node.Attribute("name");
	
	if (name)
	{
		CSkinVariableString* tmp = new CSkinVariableString;
		tmp->m_name = name;
		tmp->m_context = context;
		const TiXmlElement* valuenode = node.FirstChildElement("value");
		
		while (valuenode)
		{
			if (valuenode->FirstChild())
			{
				CSkinVariableString::ConditionLabelPair pair;
				
				if (valuenode->Attribute("condition"))
					pair.m_condition = g_infoManager.Register(valuenode->Attribute("condition"), context);
				else
					pair.m_condition = DEFAULT_VALUE;

				pair.m_label = CGUIInfoLabel(valuenode->FirstChild()->Value());
				tmp->m_conditionLabelPairs.push_back(pair);
				
				if (pair.m_condition == DEFAULT_VALUE)
					break; // Once we reach default value (without condition) break iterating
			}
			valuenode = valuenode->NextSiblingElement("value");
		}
		
		if (tmp->m_conditionLabelPairs.size() > 0)
			return tmp;
		
		delete tmp;
	}
	return NULL;
}

CSkinVariableString::CSkinVariableString()
{
}

int CSkinVariableString::GetContext() const
{
	return m_context;
}

const CStdString& CSkinVariableString::GetName() const
{
	return m_name;
}

CStdString CSkinVariableString::GetValue(bool preferImage /* = false*/, const CGUIListItem *item /* = NULL */)
{
	for (VECCONDITIONLABELPAIR::const_iterator it = m_conditionLabelPairs.begin() ; it != m_conditionLabelPairs.end(); it++)
	{
		if (it->m_condition == DEFAULT_VALUE || g_infoManager.GetBoolValue(it->m_condition, item))
		{
			if (item)
				return it->m_label.GetItemLabel(item, preferImage);
			else
				return it->m_label.GetLabel(m_context, preferImage);
		}
	}
	return "";
}
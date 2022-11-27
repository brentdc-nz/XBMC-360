#ifndef H_CGUIINCLUDES
#define H_CGUIINCLUDES

#include <map>
#include <set>

#include "utils\StdString.h"

// Forward definitions
class TiXmlElement;

namespace INFO
{
	class CSkinVariableString;
}

class CGUIIncludes
{
public:
	CGUIIncludes();
	~CGUIIncludes();

	void ClearIncludes();
	bool LoadIncludes(const CStdString &includeFile);
	void ResolveIncludes(TiXmlElement *node, const CStdString &type);
	const INFO::CSkinVariableString* CreateSkinVariable(const CStdString& name, int context);
	bool ResolveConstant(const CStdString &constant, float &value) const;
	bool LoadIncludesFromXML(const TiXmlElement *root);

private:
	bool HasIncludeFile(const CStdString &includeFile) const;
	std::map<CStdString, TiXmlElement> m_includes;
	std::map<CStdString, TiXmlElement> m_defaults;
	std::map<CStdString, TiXmlElement> m_skinvariables;
	std::map<CStdString, float> m_constants;
	std::vector<CStdString> m_files;
	typedef std::vector<CStdString>::const_iterator iFiles;
};

#endif //H_CGUIINCLUDES
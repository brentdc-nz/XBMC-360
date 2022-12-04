#include "GUIColorManager.h"
#include "utils\log.h"
#include "utils\URIUtils.h"
#include "SkinInfo.h"

CGUIColorManager g_colorManager;

CGUIColorManager::CGUIColorManager(void)
{
}

CGUIColorManager::~CGUIColorManager(void)
{
	Clear();
}

void CGUIColorManager::Clear()
{
	m_colors.clear();
}

// Load the color file in
void CGUIColorManager::Load(const CStdString &colorFile)
{
	Clear();

	// Load the global color map if it exists
	TiXmlDocument xmlDoc;
/*
	if (xmlDoc.LoadFile(PTH_IC("special://xbmc/system/colors.xml"))) // TODO
		LoadXML(xmlDoc);
*/
	// First load the default color map if it exists
	CStdString path, basePath;
	URIUtils::AddFileToFolder(g_SkinInfo.GetBaseDir(), "colors", basePath);
	URIUtils::AddFileToFolder(basePath, "defaults.xml", path);

	if (xmlDoc.LoadFile(path))
		LoadXML(xmlDoc);

	// Now the color map requested
	if (colorFile.CompareNoCase("SKINDEFAULT") == 0)
		return; // Nothing to do

	URIUtils::AddFileToFolder(basePath, colorFile, path);
	CLog::Log(LOGINFO, "Loading colors from %s", path.c_str());

	if (xmlDoc.LoadFile(path))
		LoadXML(xmlDoc);
}

bool CGUIColorManager::LoadXML(TiXmlDocument &xmlDoc)
{
	TiXmlElement* pRootElement = xmlDoc.RootElement();

	CStdString strValue = pRootElement->Value();
	if (strValue != CStdString("colors"))
	{
		CLog::Log(LOGERROR, "color file doesnt start with <colors>");
		return false;
	}

	const TiXmlElement *color = pRootElement->FirstChildElement("color");

	while (color)
	{
		if (color->FirstChild() && color->Attribute("name"))
		{
			color_t value = 0xffffffff;
			sscanf(color->FirstChild()->Value(), "%x", (unsigned int*) &value);
			CStdString name = color->Attribute("name");
			iColor it = m_colors.find(name);
			
			if (it != m_colors.end())
				(*it).second = value;
			else
				m_colors.insert(make_pair(name, value));
		}
		color = color->NextSiblingElement("color");
	}

	return true;
}

// Lookup a color and return it's hex value
color_t CGUIColorManager::GetColor(const CStdString &color) const
{
	// Look in our color map
	CStdString trimmed(color);
	trimmed.TrimLeft("= ");
	icColor it = m_colors.find(trimmed);
	
	if (it != m_colors.end())
		return (*it).second;

	// Try converting hex directly
	color_t value = 0;
	sscanf(trimmed.c_str(), "%x", &value);

	return value;
}
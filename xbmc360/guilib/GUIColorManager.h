#ifndef GUILIB_COLORMANAGER_H
#define GUILIB_COLORMANAGER_H

#include "utils\stdafx.h"
#include "tinyxml\tinyxml.h"

class TiXmlDocument;

typedef uint32_t color_t;

class CGUIColorManager
{
public:
	CGUIColorManager(void);
	virtual ~CGUIColorManager(void);

	void Load(const CStdString &colorFile);

	color_t GetColor(const CStdString &color) const;

	void Clear();

protected:
	bool LoadXML(TiXmlDocument &xmlDoc);

	std::map<CStdString, color_t> m_colors;
	typedef std::map<CStdString, color_t>::iterator iColor;
	typedef std::map<CStdString, color_t>::const_iterator icColor;
};

extern CGUIColorManager g_colorManager;

#endif //GUILIB_COLORMANAGER_H

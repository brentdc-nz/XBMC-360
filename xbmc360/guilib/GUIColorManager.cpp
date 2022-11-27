#include "GUIColorManager.h"

CGUIColorManager g_colorManager;

// TODO - Load color file!

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
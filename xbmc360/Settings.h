#ifndef H_CSETTINGS
#define H_CSETTINGS

#include "utils\StdString.h"
#include "guilib\tinyxml\tinyxml.h"

#define DEFAULT_SKIN "Project Mayhem III"

class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

	void Save() const;
	bool SaveSettings(const CStdString& strSettingsFile) const;
	bool Load();
	bool LoadSettings(const CStdString& strSettingsFile);

protected:
	void GetInteger(const TiXmlElement* pRootElement, const CStdString& strTagName, int& iValue, const int iDefault, const int iMin, const int iMax);
};

extern CSettings g_settings;

#endif //H_CSETTINGS
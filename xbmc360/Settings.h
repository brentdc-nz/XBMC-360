#ifndef H_CSETTINGS
#define H_CSETTINGS

#include "utils\StdString.h"

class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

	void Save() const;
	bool SaveSettings(const CStdString& strSettingsFile) const;
};

extern CSettings g_settings;

#endif //H_CSETTINGS
#ifndef H_CGUISETTINGS
#define H_CGUISETTINGS

#include "utils\StdString.h"
#include <map>

// base class for all settings types
class CSetting
{
public:
	CSetting(const char *strSetting) {m_strSetting = strSetting;};
	~CSetting() {};

private:
	CStdString m_strSetting;
};

class CSettingString : public CSetting
{
public:
	CSettingString(const char *strSetting, const char *strData);
	~CSettingString() {};

	CStdString GetData() const { return m_strData; };

private:
	CStdString m_strData;
};

class CGUISettings
{
public:
	CGUISettings(void);
	virtual ~CGUISettings(void);

	void AddString(const char *strSetting, const char *strData);
	CStdString GetString(const char *strSetting);

	void Clear();

private:
	typedef std::map<CStdString, CSetting*>::iterator mapIter;
	std::map<CStdString, CSetting*> m_settingsMap;
};

extern class CGUISettings g_guiSettings;

#endif //H_CGUISETTINGS
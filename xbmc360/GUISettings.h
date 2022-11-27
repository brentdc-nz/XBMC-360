#ifndef H_CGUISETTINGS
#define H_CGUISETTINGS

#include "utils\StdString.h"
#include "guilib\tinyxml\tinyxml.h"
#include "Settings.h"

#include <map>
#include <vector>

#define SPIN_CONTROL_INT        3
#define SPIN_CONTROL_INT_PLUS	4
#define SPIN_CONTROL_TEXT		5

class CSettingsCategory
{
public:
	CSettingsCategory(const char *strCategory, DWORD dwLabelID)
	{
		m_strCategory = strCategory;
		m_dwLabelID = dwLabelID;
	}
	~CSettingsCategory() {};

	CStdString m_strCategory;
	DWORD m_dwLabelID;
};

typedef std::vector<CSettingsCategory *> vecSettingsCategory;

class CSettingsGroup
{
public:
	CSettingsGroup(DWORD dwGroupID, DWORD dwLabelID)
	{
		m_dwGroupID = dwGroupID;
		m_dwLabelID = dwLabelID;
	}
	~CSettingsGroup()
	{
		for (unsigned int i = 0; i < m_vecCategories.size(); i++)
			delete m_vecCategories[i];

		m_vecCategories.clear();
	};

	void AddCategory(const char *strCategory, DWORD dwLabelID)
	{
		CSettingsCategory *pCategory = new CSettingsCategory(strCategory, dwLabelID);
		
		if (pCategory)
			m_vecCategories.push_back(pCategory);
	}

	void GetCategories(vecSettingsCategory &vecCategories)
	{
		vecCategories.clear();
		for (unsigned int i = 0; i < m_vecCategories.size(); i++)
		vecCategories.push_back(m_vecCategories[i]);
	};
	DWORD GetLabelID() { return m_dwLabelID; };
	DWORD GetGroupID() { return m_dwGroupID; };

private:
	vecSettingsCategory m_vecCategories;
	DWORD m_dwGroupID;
	DWORD m_dwLabelID;
};

// base class for all settings types
class CSetting
{
public:
	CSetting(int iOrder, const char *strSetting, int iLabel, int iControlType) { m_iOrder = iOrder; m_strSetting = strSetting; m_iLabel = iLabel; m_iControlType = iControlType;};
	~CSetting() {};

	virtual void FromString(const CStdString &strValue) {};
	virtual CStdString ToString() { return ""; };
	const char *GetSetting() { return m_strSetting.c_str(); };
	int GetControlType() { return m_iControlType; };
	int GetLabel() { return m_iLabel; };
	int GetOrder() const { return m_iOrder; };

private:
	int m_iControlType;
	int m_iLabel;
	int m_iOrder;
	CStdString m_strSetting;
};

class CSettingString : public CSetting
{
public:
	CSettingString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType);
	~CSettingString() {};

	virtual void FromString(const CStdString &strValue);
	virtual CStdString ToString();
	void SetData(const char *strData) { m_strData = strData; };
	CStdString GetData() const { return m_strData; };

private:
	CStdString m_strData;
};

class CSettingInt : public CSetting
{
public:
	CSettingInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType);
	~CSettingInt() {};

	virtual void FromString(const CStdString &strValue);
	virtual CStdString ToString();

	void SetData(int iData) { m_iData = iData; if (m_iData < m_iMin) m_iData = m_iMin; if (m_iData > m_iMax) m_iData = m_iMax;};
	int GetData() const { return m_iData; };

	int m_iMin;
	int m_iStep;
	int m_iMax;
	int m_iFormat;
	int m_iLabelMin;
	CStdString m_strFormat;

protected:
	int m_iData;
};

typedef std::vector<CSetting *> vecSettings;

class CGUISettings
{
public:
	CGUISettings(void);
	virtual ~CGUISettings(void);

	void AddGroup(DWORD dwGroupID, DWORD dwLabelID);
	void AddCategory(DWORD dwGroupID, const char *strSetting, DWORD dwLabelID);
	CSettingsGroup *GetGroup(DWORD dwWindowID);

	void AddString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType);
	CStdString GetString(const char *strSetting);
	void SetString(const char *strSetting, const char *strData);

	void AddInt(int iOrder, const char *strSetting, int iLabel, int fSetting, int iMin, int iStep, int iMax, int iControlType);
	int GetInt(const char *strSetting);

	void GetSettingsGroup(const char *strGroup, vecSettings &settings);
	CSetting *GetSetting(const char *strSetting);

	void LoadXML(TiXmlElement *pRootElement);
	void SaveXML(TiXmlNode *pRootNode);

	// m_LookAndFeelResolution holds the real gui resolution,
	// also when g_guiSettings.GetInt("videoscreen.resolution") is set to AUTORES
	RESOLUTION m_LookAndFeelResolution;

	void Clear();

private:
	typedef std::map<CStdString, CSetting*>::iterator mapIter;
	std::map<CStdString, CSetting*> m_settingsMap;
	std::vector<CSettingsGroup *> m_settingsGroups;
};

extern class CGUISettings g_guiSettings;

#endif //H_CGUISETTINGS
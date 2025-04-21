#pragma once

#include "utils\InfoLoader.h"
#include <map>

#define WEATHER_LABEL_LOCATION     10
#define WEATHER_IMAGE_CURRENT_ICON 21
#define WEATHER_LABEL_CURRENT_COND 22
#define WEATHER_LABEL_CURRENT_TEMP 23
#define WEATHER_LABEL_CURRENT_FEEL 24
#define WEATHER_LABEL_CURRENT_UVID 25
#define WEATHER_LABEL_CURRENT_WIND 26
#define WEATHER_LABEL_CURRENT_DEWP 27
#define WEATHER_LABEL_CURRENT_HUMI 28

class CDayForcast
{
public:
	CDayForcast(CStdString strOverview, CStdString strIcon, CStdString strDay, CStdString strTmpLow, CStdString strTmpHigh)
	{
		m_strOverview = strOverview;
		m_strIcon = strIcon;
		m_strDay = strDay;
		m_strTmpLow = strTmpLow;
		m_strTmpHigh = strTmpHigh;
	}
	~CDayForcast() {}

	CStdString GetOverview() { return m_strOverview; };
	CStdString GetIcon() { return m_strIcon; };
	CStdString GetDay() { return m_strDay; };
	CStdString GetTmpLow() { return m_strTmpLow; };
	CStdString GetTmpHigh() { return m_strTmpHigh; };

private:
	CStdString m_strOverview;
	CStdString m_strIcon;
	CStdString m_strDay;
	CStdString m_strTmpLow;
	CStdString m_strTmpHigh;
};

class CWeather : public CInfoLoader
{
public:
	CWeather(void);
	virtual ~CWeather(void);

	bool GetSearchResults(const CStdString &strSearch, CStdString &strResult);
	CStdString GetLocation(int iLocation);

	const CStdString &GetLastUpdateTime() const { return m_strLastUpdateTime; };
	bool IsFetched();
	void Reset();

	void SetCurSelection(int iSelection) { m_iCurWeather = iSelection; };
	int GetCurSelection() const { return m_iCurWeather; };

	CStdString GetLocationCoordinates(const CStdString &strCordAndName) const;
	CStdString GetLocationName(const CStdString &strCordAndName) const;
	std::vector<CDayForcast> GetForcastDays() { return m_vecDayForcast; }

protected:
	virtual bool DoWork();
	virtual CStdString TranslateInfo(int info) const;
	virtual CStdString BusyInfo(int info) const;

private:
	bool LoadWeatherData(const std::string &strJson);
	void LocalizeOverview(CStdString &str);
	void LocalizeOverviewToken(CStdString &str);
	CStdString LocalizeDay(int iDayNum);
	void LoadLocalizedToken();
	int ConvertSpeed(int iSpeed);
	std::map<CStdString, int> m_localizedTokens;
	typedef std::map<CStdString, int>::const_iterator ilocalizedTokens;

	CStdString m_location[3]; // UI can store hold 3

	// Current weather
	CStdString m_strCurrentCond;
	CStdString m_strCurrentIcon;
	CStdString m_strCurrentTemp;
	CStdString m_strCurrentFeelsLike;
	CStdString m_strCurrentUVIndex;
	CStdString m_strCurrentWind;
	CStdString m_strCurrentDewPoint;
	CStdString m_strCurrentHumidity;

	// Future forcast
	std::vector<CDayForcast> m_vecDayForcast;

	unsigned int m_iCurWeather;
	CStdString m_strLastUpdateTime;
};

extern CWeather g_weatherManager;
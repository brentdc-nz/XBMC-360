#include "Weather.h"
#include "Application.h"
#include "guilib\tinyjson\tinyjson.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\dialogs\GUIDialogSelect.h"
#include "guilib\dialogs\GUIDialogProgress.h"
#include "libraries\httpclient\httpclient.h"
#include "GUISettings.h"
#include "Temperature.h"
#include "utils\POUtils.h"
#include "guilib\XMLUtils.h"
#include "utils\CharsetConverter.h"
#include "guilib\LocalizeStrings.h"
#include "LangInfo.h"

#define NUM_DAYS 5
#define MAX_LOCATION 3
#define WEATHER_API_KEY "d0b141a33e0548ee83774830251704"
#define WEATHER_BASE_PATH "D:\\media\\weather\\"

#define LOCALIZED_TOKEN_FIRSTID		370
#define LOCALIZED_TOKEN_LASTID		395
#define LOCALIZED_TOKEN_FIRSTID2	1396
#define LOCALIZED_TOKEN_LASTID2		1450 

using namespace std;
using namespace tinyjson;

CWeather g_weatherManager;

CWeather::CWeather(void) : CInfoLoader(30 * 60 * 1000) // 30 minutes
{
	Reset();
}

CWeather::~CWeather(void)
{
	m_localizedTokens.clear();
}

void CWeather::Reset()
{
	m_strCurrentCond = "";
	m_strCurrentIcon = "";
	m_strCurrentTemp = "";
	m_strCurrentFeelsLike = "";
	m_strCurrentUVIndex = "";
	m_strCurrentWind = "";
	m_strCurrentDewPoint = "";
	m_strCurrentHumidity = "";

	m_vecDayForcast.clear();

	for (int i = 0; i < MAX_LOCATION; i++)
		m_location[i] = "";
}

bool CWeather::GetSearchResults(const CStdString &strSearch, CStdString &strResult) // TODO: Check for error response from API
{
	CGUIDialogSelect *pDlgSelect = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
	CGUIDialogProgress *pDlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

	if (pDlgProgress)
	{
		pDlgProgress->SetHeading(410); // "Accessing api.weatherapi.com" // TODO: Update strings file
		pDlgProgress->SetLine(0, 194); // "Searching"
		pDlgProgress->SetLine(1, strSearch);
		pDlgProgress->SetLine(2, "");
		pDlgProgress->StartModal();
		pDlgProgress->Progress();
	}

	// Call API for our search
	string strJsonResponse;
	CStdString strUrl;

	strUrl.Format("https://api.weatherapi.com/v1/search.json?key=%s&q=%s", WEATHER_API_KEY, CHTTPClient::UrlEncode(strSearch));

	if (!CHTTPClient::DownloadFile(strUrl, strJsonResponse))
	{
		if (pDlgProgress)
			pDlgProgress->Close();

		return false;
	}

	if (!pDlgSelect)
	{
		if (pDlgProgress)
			pDlgProgress->Close();

		return false;
	}

	pDlgSelect->SetHeading(396); // "Select Location"
	pDlgSelect->Reset();

	tinyjson::TinyJson json;

	if (!json.ReadJson(strJsonResponse))
	{
		CLog::Log(LOGERROR, "WEATHER: Unable to get data - Invalid json");
		return false;
	}

	if(json.Get<xobject>("error").Count() >= 1)
	{
		CLog::Log(LOGERROR, "WEATHER: Unable to get data - Error received in json data");
		return false;
	}

	tinyjson::xarray vecLocationsFound = json.GetRootArray();
	std::vector<std::string> vecCoordinatesList;

	for (int i = 0; i < vecLocationsFound.Count(); i++)
	{
		vecLocationsFound.Enter(i);

		CStdString strLocation, strCoordinates;

		strLocation.Format("%s, %s",
			vecLocationsFound.Get<std::string>("name"),
			vecLocationsFound.Get<std::string>("country"));

		strCoordinates.Format("%s:%s",
			vecLocationsFound.Get<std::string>("lat"),
			vecLocationsFound.Get<std::string>("lon"));

		vecCoordinatesList.push_back(strCoordinates);
		pDlgSelect->Add(strLocation);
	}

	if (pDlgProgress)
		pDlgProgress->Close();

	pDlgSelect->EnableButton(TRUE);
	pDlgSelect->SetButtonLabel(222); // 'Cancel'
	pDlgSelect->DoModal();

	int iSelectedIndex = pDlgSelect->GetSelectedLabel();

	if (iSelectedIndex < 0)
	{
		if (pDlgSelect->IsButtonPressed())
		{
			pDlgSelect->Close(); // Close the select dialog and return to weather settings
			return true;
		}
	}
	else
	{
		if (iSelectedIndex < (int)vecCoordinatesList.size())
		{
			// Put into our results for the settings
			strResult.Format("%s~%s", vecCoordinatesList[iSelectedIndex].c_str(), pDlgSelect->GetSelectedLabelText().c_str());
		}
	}

	return true;
}

CStdString CWeather::GetLocation(int iLocation)
{
	if (iLocation < 0 || iLocation >= MAX_LOCATION)
		return "";

	if (m_location[iLocation].IsEmpty())
	{
		CStdString strSetting;
		strSetting.Format("weather.location%i", iLocation + 1);
		m_location[iLocation] = GetLocationName(g_guiSettings.GetString(strSetting));
	}

	return m_location[iLocation];
}

bool CWeather::DoWork()
{
	if (!g_application.getNetwork().IsAvailable())
		return false;

	// Download our weather
	CLog::Log(LOGINFO, "WEATHER: Downloading weather");

	// Get the location for the current selection
	CStdString strLocSetting;
	strLocSetting.Format("weather.location%i", GetCurSelection() + 1);

	CStdString strUrl;
	strUrl.Format("https://api.weatherapi.com/v1/forecast.json?key=%s&q=%s&days=%d",
		WEATHER_API_KEY,
		GetLocationCoordinates(g_guiSettings.GetString(strLocSetting)),
		NUM_DAYS);

	string strJsonResponse;

	if (CHTTPClient::DownloadFile(strUrl, strJsonResponse))
	{
		CLog::Log(LOGINFO, "WEATHER: Weather download successful");

		// WIP Icon extract here //

		LoadWeatherData(strJsonResponse);

		// Now send a message that we're done
		CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_WEATHER_FETCHED);
		g_windowManager.SendThreadMessage(msg);
	}
	else
		CLog::Log(LOGERROR, "WEATHER: Weather download failed!");

	return true;
}

bool CWeather::LoadWeatherData(const string &strJson)
{
	// Load in our tokens if necessary
	if (!m_localizedTokens.size())
		LoadLocalizedToken();

	// Make sure our forcast days are cleared
	m_vecDayForcast.clear();

	tinyjson::TinyJson json;

	if (!json.ReadJson(strJson))
	{
		CLog::Log(LOGERROR, "WEATHER: Unable to get data - Invalid json");
		return false;
	}

	if(json.Get<xobject>("error").Count() >= 1)
	{
		CLog::Log(LOGERROR, "WEATHER: Unable to get data - Error received in json data");
		return false;
	}

	// Parse all the info we need

	// Location

	xobject objLocationData = json.Get<xobject>("location");

	if(objLocationData.Enter(0))
		m_location[m_iCurWeather].Format("%s, %s", objLocationData.Get<string>("name"), objLocationData.Get<string>("country")); 

	// Current weather

	xobject objCurrentdata = json.Get<xobject>("current");
	
	if(objCurrentdata.Enter(0))
	{
		// Current temp
		CTemperature temp = CTemperature::CreateFromCelsius(objCurrentdata.Get<double>("temp_c"));
		m_strCurrentTemp.Format("%2.0f", temp.ToLocale());

		// Current 'Feels Like'
		CTemperature tempFlik = CTemperature::CreateFromCelsius(objCurrentdata.Get<double>("feelslike_c"));
		m_strCurrentFeelsLike.Format("%2.0f", tempFlik.ToLocale());

		// Current UV index
		m_strCurrentUVIndex = objCurrentdata.Get<string>("uv");

		// Current wind
		int iWindSpeedKph = objCurrentdata.Get<int>("wind_kph");

		if (iWindSpeedKph <=  1)
			m_strCurrentWind =  g_localizeStrings.Get(1410); // "Calm"
		else
			m_strCurrentWind.Format("%s %s %s %i %s",
				g_localizeStrings.Get(407)/*"Wind From"*/, objCurrentdata.Get<string>("wind_dir"),
				g_localizeStrings.Get(408)/*"Wind At"*/, ConvertSpeed(iWindSpeedKph), g_langInfo.GetSpeedUnitString().c_str());

		// Current dew point
		CTemperature tempDewPoint = CTemperature::CreateFromCelsius(objCurrentdata.Get<double>("dewpoint_c"));
		m_strCurrentDewPoint.Format("%2.0f", tempDewPoint.ToLocale());

		// Current Humidity
		m_strCurrentHumidity = objCurrentdata.Get<string>("humidity");

		// Used for icon below
		int iIsDayTime = objCurrentdata.Get<int>("is_day");

		// Nested Condition
		xobject objCondition = objCurrentdata.Get<xobject>("condition");

		if (objCondition.Enter(0))
		{
			m_strCurrentCond = objCondition.Get<string>("text");
			LocalizeOverview(m_strCurrentCond);

			if(iIsDayTime == 1)
				m_strCurrentIcon.Format("%sd%s.png",WEATHER_BASE_PATH, objCondition.Get<string>("code"));
			else
				m_strCurrentIcon.Format("%sn%s.png",WEATHER_BASE_PATH, objCondition.Get<string>("code"));
		}
	}

	// Future forcast

	xobject objForcastData = json.Get<xobject>("forecast");

	for (int i = 0; i < objForcastData.Count(); i++)
	{
		// Do we want the current day in the forcast overview?
		if(i == 0)
			continue;

		objForcastData.Enter(i);

		// The info we need for the day
		CStdString strOverview("");
		CStdString strIcon("");
		CStdString strDay("");
		CStdString strTempMin("");
		CStdString strTempMax("");

		// Convert from DB time format to day number
		CDateTime objDayConvert;
		objDayConvert.SetFromDBDate(objForcastData.Get<string>("date"));

		// Now convert the day number to localized day string
		strDay = LocalizeDay(objDayConvert.GetDayOfWeek());

		// Nested Day
		xobject objForcastDay = objForcastData.Get<xobject>("day");

		if (objForcastDay.Enter(0))
		{
			// Min temp
			CTemperature tempMin = CTemperature::CreateFromCelsius(objForcastDay.Get<double>("mintemp_c"));
			strTempMin.Format("%2.0f", tempMin.ToLocale());

			// Max temp
			CTemperature tempMax = CTemperature::CreateFromCelsius(objForcastDay.Get<double>("maxtemp_c"));
			strTempMax.Format("%2.0f", tempMax.ToLocale());

			// Nested Condition
			xobject objForcastDayCond = objForcastDay.Get<xobject>("condition");

			if (objForcastDayCond.Enter(0))
			{
				strOverview = objForcastDayCond.Get<string>("text");
				strIcon.Format("%sd%s.png",WEATHER_BASE_PATH, objForcastDayCond.Get<string>("code"));
			}
		}
		
		m_vecDayForcast.push_back(CDayForcast(strOverview, strIcon, strDay, strTempMin, strTempMax));
	}

	// Use the local date/time the file is parsed... // TODO: Makes more sense to use last updated by API??
	CDateTime dtTime = CDateTime::GetCurrentDateTime();
	m_strLastUpdateTime = dtTime.GetAsLocalizedDateTime(false, false);

	return true;
}

CStdString CWeather::TranslateInfo(int info) const
{
	if (info == WEATHER_LABEL_CURRENT_COND) return m_strCurrentCond;
	else if (info == WEATHER_IMAGE_CURRENT_ICON) return m_strCurrentIcon;
	else if (info == WEATHER_LABEL_CURRENT_TEMP) return m_strCurrentTemp;
	else if (info == WEATHER_LABEL_CURRENT_FEEL) return m_strCurrentFeelsLike;
	else if (info == WEATHER_LABEL_CURRENT_UVID) return m_strCurrentUVIndex;
	else if (info == WEATHER_LABEL_CURRENT_WIND) return m_strCurrentWind;
	else if (info == WEATHER_LABEL_CURRENT_DEWP) return m_strCurrentDewPoint;
	else if (info == WEATHER_LABEL_CURRENT_HUMI) return m_strCurrentHumidity;
	else if (info == WEATHER_LABEL_LOCATION) return m_location[m_iCurWeather];
	return "";
}

CStdString CWeather::BusyInfo(int info) const
{
	if (info == WEATHER_IMAGE_CURRENT_ICON)
	{
		CStdString strBusyIcon;
		strBusyIcon.Format("%\\na.png", WEATHER_BASE_PATH);

		return strBusyIcon;
	}
	return CInfoLoader::BusyInfo(info);
}

CStdString CWeather::GetLocationCoordinates(const CStdString &strCordAndName) const
{
	// Trim the city name for coordinates
	int iSep = strCordAndName.Find("~");
	CStdString strCoordinates;

	if (iSep >= 0)
		strCoordinates = strCordAndName.Left(iSep).Trim();

	// Replace ':' with ','
	strCoordinates.Replace(':', ',');

	return strCoordinates;
}

CStdString CWeather::GetLocationName(const CStdString &strCordAndName) const
{
	CStdString strLocationName(strCordAndName);

	// Trim the coordinates for display of name
	int iSep = strCordAndName.Find("~");
	if (iSep >= 0)
		strLocationName = strCordAndName.Mid(iSep + 1).Trim();

	return strLocationName;
}

bool CWeather::IsFetched()
{
	// Call GetInfo() to make sure that we actually start up
	GetInfo(0);
	return !m_strLastUpdateTime.IsEmpty();
}

void CWeather::LoadLocalizedToken()
{
	// We load the english strings in to get our tokens

	// Try the strings PO file first
	CPODocument PODoc;
	if (PODoc.LoadFile("D:\\language\\English\\strings.po"))
	{
		int counter = 0;

		while (PODoc.GetNextEntry())
		{
			if (PODoc.GetEntryType() != ID_FOUND)
				continue;

			uint32_t id = PODoc.GetEntryID();
			PODoc.ParseEntry(ISSOURCELANG);

			if (id > LOCALIZED_TOKEN_LASTID2) break;
			
			if ((LOCALIZED_TOKEN_FIRSTID  <= id && id <= LOCALIZED_TOKEN_LASTID)  ||
				(LOCALIZED_TOKEN_FIRSTID2 <= id && id <= LOCALIZED_TOKEN_LASTID2))
			{
				if (!PODoc.GetMsgid().empty())
				{
					m_localizedTokens.insert(make_pair(PODoc.GetMsgid(), id));
					counter++;
				}
			}
		}

		CLog::Log(LOGDEBUG, "POParser: loaded %i weather tokens", counter);
		return;
	}

	CLog::Log(LOGDEBUG,
		"Weather: no PO string file available, to load English tokens, "
		"fallback to strings.xml file");

	// We load the tokens from the strings.xml file
	CStdString strLanguagePath = "D:\\language\\English\\strings.xml";
  
	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile(strLanguagePath) || !xmlDoc.RootElement())
	{
		CLog::Log(LOGERROR, "Weather: unable to load %s: %s at line %d", strLanguagePath.c_str(), xmlDoc.ErrorDesc(), xmlDoc.ErrorRow());
		return;
	}

	CStdString strEncoding;
	XMLUtils::GetEncoding(&xmlDoc, strEncoding);

	TiXmlElement* pRootElement = xmlDoc.RootElement();

	if (pRootElement->Value() != CStdString("strings"))
		return;

	const TiXmlElement *pChild = pRootElement->FirstChildElement();

	while (pChild)
	{
		CStdString strValue = pChild->Value();

		if (strValue == "string")
		{
			// Load new style language file with id as attribute
			const char* attrId = pChild->Attribute("id");

			if (attrId && !pChild->NoChildren())
			{
				int id = atoi(attrId);
				if ((LOCALIZED_TOKEN_FIRSTID <= id && id <= LOCALIZED_TOKEN_LASTID) ||
					(LOCALIZED_TOKEN_FIRSTID2 <= id && id <= LOCALIZED_TOKEN_LASTID2))
				{
					CStdString utf8Label;

				if (strEncoding.IsEmpty()) // Is language file utf8?
					utf8Label=pChild->FirstChild()->Value();
				else
					g_charsetConverter.stringCharsetToUtf8(strEncoding, pChild->FirstChild()->Value(), utf8Label);

				if (!utf8Label.IsEmpty())
					m_localizedTokens.insert(make_pair(utf8Label, id));
				}
			}
		}
		pChild = pChild->NextSiblingElement();
	}
}

void CWeather::LocalizeOverviewToken(CStdString &token)
{
	// NOTE: This routine is case-sensitive. Reason is std::less<CStdString> uses a case-sensitive
	//       operator.  Thus, some tokens may have to be duplicated in strings.xml (see drizzle vs Drizzle)
	CStdString strLocStr = "";
	
	if (!token.IsEmpty())
	{
		ilocalizedTokens i;
		i = m_localizedTokens.find(token);
		
		if (i != m_localizedTokens.end())
		{
			strLocStr = g_localizeStrings.Get(i->second);
		}
	}
	
	if (strLocStr == "")
		strLocStr = token; // If not found, let fallback
	
	token = strLocStr;
}

CStdString CWeather::LocalizeDay(int iDayNum)
{
	// Standard weekday number to locallized string
    switch(iDayNum)
	{
        case 0: return g_localizeStrings.Get(17); // Sunday
        case 1: return g_localizeStrings.Get(11); // Monday 
        case 2: return g_localizeStrings.Get(12); // Tuesday
        case 3: return g_localizeStrings.Get(13); // Wednesday
        case 4: return g_localizeStrings.Get(14); // Thursday
        case 5: return g_localizeStrings.Get(15); // Friday
        case 6: return g_localizeStrings.Get(16); // Saturday
        default: return "Invalid";
    }
}

// Input param must be kmh
int CWeather::ConvertSpeed(int iSpeed)
{
	switch (g_langInfo.GetSpeedUnit())
	{
		case CLangInfo::SPEED_UNIT_KMH:
		break;
		case CLangInfo::SPEED_UNIT_MPS:
			iSpeed=(int)(iSpeed * (1000.0 / 3600.0) + 0.5);
		break;
		case CLangInfo::SPEED_UNIT_MPH:
			iSpeed=(int)(iSpeed / (8.0 / 5.0));
		break;
		case CLangInfo::SPEED_UNIT_MPMIN:
			iSpeed=(int)(iSpeed * (1000.0 / 3600.0) + 0.5*60);
		break;
		case CLangInfo::SPEED_UNIT_FTH:
			iSpeed=(int)(iSpeed * 3280.8398888889f);
		break;
		case CLangInfo::SPEED_UNIT_FTMIN:
			iSpeed=(int)(iSpeed * 54.6805555556f);
		break;
		case CLangInfo::SPEED_UNIT_FTS:
			iSpeed=(int)(iSpeed * 0.911344f);
		break;
		case CLangInfo::SPEED_UNIT_KTS:
			iSpeed=(int)(iSpeed * 0.5399568f);
		break;
		case CLangInfo::SPEED_UNIT_INCHPS:
			iSpeed=(int)(iSpeed * 10.9361388889f);
		break;
		case CLangInfo::SPEED_UNIT_YARDPS:
			iSpeed=(int)(iSpeed * 0.3037814722f);
		break;
		case CLangInfo::SPEED_UNIT_FPF:
			iSpeed=(int)(iSpeed * 1670.25f);
		break;
		case CLangInfo::SPEED_UNIT_BEAUFORT:
		{
			float knot=(float)iSpeed * 0.5399568f; // To kts first
			if(knot<=1.0) iSpeed=0;
			if(knot>1.0 && knot<3.5) iSpeed=1;
			if(knot>=3.5 && knot<6.5) iSpeed=2;
			if(knot>=6.5 && knot<10.5) iSpeed=3;
			if(knot>=10.5 && knot<16.5) iSpeed=4;
			if(knot>=16.5 && knot<21.5) iSpeed=5;
			if(knot>=21.5 && knot<27.5) iSpeed=6;
			if(knot>=27.5 && knot<33.5) iSpeed=7;
			if(knot>=33.5 && knot<40.5) iSpeed=8;
			if(knot>=40.5 && knot<47.5) iSpeed=9;
			if(knot>=47.5 && knot<55.5) iSpeed=10;
			if(knot>=55.5 && knot<63.5) iSpeed=11;
			if(knot>=63.5 && knot<74.5) iSpeed=12;
			if(knot>=74.5 && knot<80.5) iSpeed=13;
			if(knot>=80.5 && knot<89.5) iSpeed=14;
			if(knot>=89.5) iSpeed=15;
		}
		break;

		default:
			assert(false);
	}
	return iSpeed;
}

void CWeather::LocalizeOverview(CStdString &str)
{
	CStdStringArray strWords;
	CStringUtils::SplitString(str, " ", strWords);
	str.clear();

	for (unsigned int i = 0; i < strWords.size(); i++)
	{
		LocalizeOverviewToken(strWords[i]);
		str += strWords[i] + " ";
	}

	str.TrimRight(" ");
}
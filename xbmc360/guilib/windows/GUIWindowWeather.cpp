#include "GUIWindowWeather.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"
#include "guilib\GUIMessage.h"
#include "utils\Weather.h"
#include "LangInfo.h"
#include "guilib\GUIImage.h"
#include "utils\URIUtils.h"

#define CONTROL_BTNREFRESH             2
#define CONTROL_SELECTLOCATION         3
#define CONTROL_LABELUPDATED          11
#define CONTROL_IMAGELOGO            101

#define CONTROL_STATICTEMP           223
#define CONTROL_STATICFEEL           224
#define CONTROL_STATICUVID           225
#define CONTROL_STATICWIND           226
#define CONTROL_STATICDEWP           227
#define CONTROL_STATICHUMI           228

#define CONTROL_LABELD0DAY            31
#define CONTROL_LABELD0HI             32
#define CONTROL_LABELD0LOW            33
#define CONTROL_LABELD0GEN            34
#define CONTROL_IMAGED0IMG            35

#define MAX_LOCATION                   3
#define LOCALIZED_TOKEN_FIRSTID      370
#define LOCALIZED_TOKEN_LASTID       395

CGUIWindowWeather::CGUIWindowWeather(void)
	: CGUIWindow(WINDOW_WEATHER, "MyWeather.xml")
{
	m_iCurWeather = 0;
}

CGUIWindowWeather::~CGUIWindowWeather(void)
{
}

bool CGUIWindowWeather::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();
			
			if (iControl == CONTROL_BTNREFRESH)
			{
				Refresh(); // Refresh clicked so do a complete update
			}
			else if (iControl == CONTROL_SELECTLOCATION)
			{
				CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), CONTROL_SELECTLOCATION);
        
				g_windowManager.SendMessage(msg);
				m_iCurWeather = msg.GetParam1();

				CStdString strLabel = g_weatherManager.GetLocation(m_iCurWeather);

				int iPos = strLabel.ReverseFind(", ");
				
				if (iPos)
				{
					CStdString strLabel2(strLabel);
					strLabel = strLabel2.substr(0,iPos);
				}

				SET_CONTROL_LABEL(CONTROL_SELECTLOCATION, strLabel);
				Refresh();
			}
		}
		break;

		case GUI_MSG_NOTIFY_ALL:
		if (message.GetParam1() == GUI_MSG_WINDOW_RESET)
		{
			g_weatherManager.Reset();
			return true;
		}
		else if (message.GetParam1() == GUI_MSG_WEATHER_FETCHED)
		{
			UpdateLocations();
			SetProperties();
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}

void CGUIWindowWeather::OnInitWindow()
{
	// Call UpdateButtons() so that we start with our initial stuff already present
	UpdateButtons();
	UpdateLocations();
	CGUIWindow::OnInitWindow();
}

void CGUIWindowWeather::UpdateLocations()
{
	if (!IsActive()) return;

	CGUIMessage msg(GUI_MSG_LABEL_RESET, GetID(), CONTROL_SELECTLOCATION);
	g_windowManager.SendMessage(msg);
	CGUIMessage msg2(GUI_MSG_LABEL_ADD, GetID(), CONTROL_SELECTLOCATION);

	for (unsigned int i = 0; i < MAX_LOCATION; i++)
	{
		CStdString strLabel = g_weatherManager.GetLocation(i);
		
		if (strLabel.size() > 1) // Got the location string yet?
		{
			int iPos = strLabel.ReverseFind(", ");
			
			if (iPos)
			{
				CStdString strLabel2(strLabel);
				strLabel = strLabel2.substr(0,iPos);
			}

			msg2.SetParam1(i);
			msg2.SetLabel(strLabel);
			g_windowManager.SendMessage(msg2);
		}
		else
		{
			strLabel.Format("Location %i", i + 1);

			msg2.SetLabel(strLabel);
			msg2.SetParam1(i);
			g_windowManager.SendMessage(msg2);
		}
		if (i == m_iCurWeather)
			SET_CONTROL_LABEL(CONTROL_SELECTLOCATION, strLabel);
	}

	CONTROL_SELECT_ITEM(CONTROL_SELECTLOCATION, m_iCurWeather);
}

void CGUIWindowWeather::UpdateButtons()
{
	CONTROL_ENABLE(CONTROL_BTNREFRESH);

	SET_CONTROL_LABEL(CONTROL_BTNREFRESH, 184); // Refresh

	SET_CONTROL_LABEL(WEATHER_LABEL_LOCATION, g_weatherManager.GetLocation(m_iCurWeather));
	SET_CONTROL_LABEL(CONTROL_LABELUPDATED, g_weatherManager.GetLastUpdateTime());

	SET_CONTROL_LABEL(WEATHER_LABEL_CURRENT_COND, g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_COND));
	SET_CONTROL_LABEL(WEATHER_LABEL_CURRENT_TEMP, g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_TEMP) + g_langInfo.GetTempUnitString());
	SET_CONTROL_LABEL(WEATHER_LABEL_CURRENT_FEEL, g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_FEEL) + g_langInfo.GetTempUnitString());
	SET_CONTROL_LABEL(WEATHER_LABEL_CURRENT_UVID, g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_UVID));
	SET_CONTROL_LABEL(WEATHER_LABEL_CURRENT_WIND, g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_WIND));
	SET_CONTROL_LABEL(WEATHER_LABEL_CURRENT_DEWP, g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_DEWP) + g_langInfo.GetTempUnitString());
	SET_CONTROL_LABEL(WEATHER_LABEL_CURRENT_HUMI, g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_HUMI));

	CGUIImage *pImage = (CGUIImage *)GetControl(WEATHER_IMAGE_CURRENT_ICON);
	if (pImage) pImage->SetFileName(g_weatherManager.GetInfo(WEATHER_IMAGE_CURRENT_ICON));

	// Static labels
	SET_CONTROL_LABEL(CONTROL_STATICTEMP, 401);  // Temperature
	SET_CONTROL_LABEL(CONTROL_STATICFEEL, 402);  // Feels Like
	SET_CONTROL_LABEL(CONTROL_STATICUVID, 403);  // UV Index
	SET_CONTROL_LABEL(CONTROL_STATICWIND, 404);  // Wind
	SET_CONTROL_LABEL(CONTROL_STATICDEWP, 405);  // Dew Point
	SET_CONTROL_LABEL(CONTROL_STATICHUMI, 406);  // Humidity

	std::vector<CDayForcast> vecForcastDays = g_weatherManager.GetForcastDays();

	// We set to store NUM_DAYS in vecForcastDays inside CWeather
	for (int i = 0; i < (int)vecForcastDays.size(); i++)
	{
		SET_CONTROL_LABEL(CONTROL_LABELD0DAY + (i*10), vecForcastDays[i].GetDay());
		SET_CONTROL_LABEL(CONTROL_LABELD0HI + (i*10), vecForcastDays[i].GetTmpHigh() + g_langInfo.GetTempUnitString());
		SET_CONTROL_LABEL(CONTROL_LABELD0LOW + (i*10), vecForcastDays[i].GetTmpLow() + g_langInfo.GetTempUnitString());
		SET_CONTROL_LABEL(CONTROL_LABELD0GEN + (i*10), vecForcastDays[i].GetOverview());
		
		pImage = (CGUIImage *)GetControl(CONTROL_IMAGED0IMG + (i * 10));

		if (pImage)
			pImage->SetFileName(vecForcastDays[i].GetIcon());
	}
}

void CGUIWindowWeather::FrameMove()
{
	// Update our controls
	UpdateButtons();

	CGUIWindow::FrameMove();
}

// Do a complete download, parse and update
void CGUIWindowWeather::Refresh()
{
	g_weatherManager.SetCurSelection(m_iCurWeather);
	g_weatherManager.Refresh();
}

void CGUIWindowWeather::SetProperties()
{
	// Current weather
	SetProperty("Location", g_weatherManager.GetLocation(m_iCurWeather));
	SetProperty("LocationIndex", int(m_iCurWeather + 1));
	CStdString strSetting;
	strSetting.Format("weather.location%i", m_iCurWeather + 1);
	SetProperty("Location", g_weatherManager.GetLocation(m_iCurWeather));
	SetProperty("Updated", g_weatherManager.GetLastUpdateTime());
	SetProperty("Current.ConditionIcon", g_weatherManager.GetInfo(WEATHER_IMAGE_CURRENT_ICON));
	SetProperty("Current.Condition", g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_COND));
	SetProperty("Current.Temperature", g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_TEMP));
	SetProperty("Current.FeelsLike", g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_FEEL));
	SetProperty("Current.UVIndex", g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_UVID));
	SetProperty("Current.Wind", g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_WIND));
	SetProperty("Current.DewPoint", g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_DEWP));
	SetProperty("Current.Humidity", g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_HUMI));
	
	// We use the icons code number for fanart as it's the safest way
	CStdString fanartcode = URIUtils::GetFileName(g_weatherManager.GetInfo(WEATHER_IMAGE_CURRENT_ICON));
	URIUtils::RemoveExtension(fanartcode);
	SetProperty("Current.FanartCode", fanartcode);

	// Future forcast
	CStdString strDay;
	std::vector<CDayForcast> vecForcastDays = g_weatherManager.GetForcastDays();

	// We set to store NUM_DAYS in vecForcastDays inside CWeather
	for (int i = 0; i < (int)vecForcastDays.size(); i++)
	{
		strDay.Format("Day%i.", i);
		SetProperty(strDay + "Title", vecForcastDays[i].GetDay());
		SetProperty(strDay + "HighTemp", vecForcastDays[i].GetTmpHigh());
		SetProperty(strDay + "LowTemp", vecForcastDays[i].GetTmpLow());
		SetProperty(strDay + "Outlook", vecForcastDays[i].GetOverview());
		SetProperty(strDay + "OutlookIcon", vecForcastDays[i].GetIcon());
		
		fanartcode = URIUtils::GetFileName(vecForcastDays[i].GetIcon());
		URIUtils::RemoveExtension(fanartcode);
		SetProperty(strDay + "FanartCode", fanartcode);
	}
}
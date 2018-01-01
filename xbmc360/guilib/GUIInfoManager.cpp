#include "GUIInfoManager.h"
#include "GUIWindowManager.h"
#include "..\utils\stdafx.h"
#include "..\utils\log.h"
#include "..\Application.h"
#include "LocalizeStrings.h"
#include <vector>

using namespace std;

CGUIInfoManager g_infoManager;

void GUIInfo::SetInfoFlag(uint32_t flag)
{
	assert(flag >= (1 << 24));
	m_data1 |= flag;
}

uint32_t GUIInfo::GetInfoFlag() const
{
	// we strip out the bottom 24 bits, where we keep data
	// and return the flag only
	return m_data1 & 0xff000000;
}

uint32_t GUIInfo::GetData1() const
{
	// we strip out the top 8 bits, where we keep flags
	// and return the unflagged data
	return m_data1 & ((1 << 24) -1);
}

int GUIInfo::GetData2() const
{
	return m_data2;
}

CGUIInfoManager::CGUIInfoManager(void)
{
}

CGUIInfoManager::~CGUIInfoManager(void)
{
}

/// \brief Translates a string as given by the skin into an int that we use for more
/// efficient retrieval of data. Can handle combined strings on the form
/// Player.Caching + VideoPlayer.IsFullscreen (Logical and)
/// Player.HasVideo | Player.HasAudio (Logical or)
int CGUIInfoManager::TranslateString(const CStdString &condition)
{
	// translate $LOCALIZE as required
	CStdString strCondition;//(CGUIInfoLabel::ReplaceLocalize(condition));
	strCondition = condition;//CGUIInfoLabel::ReplaceAddonStrings(strCondition);

	if (strCondition.find_first_of("|") != strCondition.npos ||
		strCondition.find_first_of("+") != strCondition.npos ||
		strCondition.find_first_of("[") != strCondition.npos ||
		strCondition.find_first_of("]") != strCondition.npos)
	{
		// Have a boolean expression
		// Check if this was added before
		vector<CCombinedValue>::iterator it;
		for(it = m_CombinedValues.begin(); it != m_CombinedValues.end(); it++)
		{
			if(strCondition.CompareNoCase(it->m_info) == 0)
				return it->m_id;
		}
		return TranslateBooleanExpression(strCondition);
	}
	//Just single command.
	return TranslateSingleString(strCondition);
}

/// \brief Translates a string as given by the skin into an int that we use for more
/// efficient retrieval of data.
int CGUIInfoManager::TranslateSingleString(const CStdString &strCondition)
{
	// trim whitespace, and convert to lowercase
	CStdString strTest = strCondition;
	strTest.TrimLeft(" \t\r\n");
	strTest.TrimRight(" \t\r\n");
	if (strTest.IsEmpty()) return 0;

	bool bNegate = strTest[0] == '!';
	int ret = 0;

	if(bNegate)
		strTest.Delete(0, 1);

	CStdString original(strTest);
	strTest.ToLower();

	CStdString strCategory = strTest.Left(strTest.Find("."));

	// translate conditions...
	if (strTest.Equals("false") || strTest.Equals("no") || strTest.Equals("off")) 
		ret = SYSTEM_ALWAYS_FALSE;
	else if (strTest.Equals("true") || strTest.Equals("yes") || strTest.Equals("on")) 
		ret = SYSTEM_ALWAYS_TRUE;

	else if (strCategory.Equals("system"))
	{
		if (strTest.Equals("system.date")) 
			ret = SYSTEM_DATE;
		else if (strTest.Equals("system.time"))
			ret = SYSTEM_TIME;
	}

	else if (strCategory.Equals("player"))
	{
		if (strTest.Equals("player.hasmedia"))
			ret = PLAYER_HAS_MEDIA;
	}

	else if (strCategory.Equals("control"))
	{
		if (strTest.Left(17).Equals("control.hasfocus("))
		{
			int controlID = atoi(strTest.Mid(17, strTest.GetLength() - 18).c_str());
			if (controlID)
			{
				return AddMultiInfo(GUIInfo(bNegate ? -CONTROL_HAS_FOCUS : CONTROL_HAS_FOCUS, controlID, 0));
			}
		}
	}

	return bNegate ? -ret : ret;
}

int CGUIInfoManager::GetOperator(const char ch)
{
  if (ch == '[')
    return 5;
  else if (ch == ']')
    return 4;
  else if (ch == '!')
    return OPERATOR_NOT;
  else if (ch == '+')
    return OPERATOR_AND;
  else if (ch == '|')
    return OPERATOR_OR;
  else
    return 0;
}

bool CGUIInfoManager::EvaluateBooleanExpression(const CCombinedValue &expression, bool &result, int contextWindow)
{
  // stack to save our bool state as we go
  stack<bool> save;

  for (list<int>::const_iterator it = expression.m_postfix.begin(); it != expression.m_postfix.end(); ++it)
  {
    int expr = *it;
    if (expr == -OPERATOR_NOT)
    { // NOT the top item on the stack
      if (save.size() < 1) return false;
      bool expr = save.top();
      save.pop();
      save.push(!expr);
    }
    else if (expr == -OPERATOR_AND)
    { // AND the top two items on the stack
      if (save.size() < 2) return false;
      bool right = save.top(); save.pop();
      bool left = save.top(); save.pop();
      save.push(left && right);
    }
    else if (expr == -OPERATOR_OR)
    { // OR the top two items on the stack
      if (save.size() < 2) return false;
      bool right = save.top(); save.pop();
      bool left = save.top(); save.pop();
      save.push(left || right);
    }
    else  // operator
      save.push(GetBool(expr));
  }
  if (save.size() != 1) return false;
  result = save.top();
  return true;
}

int CGUIInfoManager::TranslateBooleanExpression(const CStdString &expression)
{
  CCombinedValue comb;
  comb.m_info = expression;
  comb.m_id = COMBINED_VALUES_START + m_CombinedValues.size();

  // operator stack
  stack<char> save;

  CStdString operand;

  for (unsigned int i = 0; i < expression.size(); i++)
  {
    if (GetOperator(expression[i]))
    {
      // cleanup any operand, translate and put into our expression list
      if (!operand.IsEmpty())
      {
        int iOp = TranslateSingleString(operand);
        if (iOp)
          comb.m_postfix.push_back(iOp);
        operand.clear();
      }
      // handle closing parenthesis
      if (expression[i] == ']')
      {
        while (save.size())
        {
          char oper = save.top();
          save.pop();

          if (oper == '[')
            break;

          comb.m_postfix.push_back(-GetOperator(oper));
        }
      }
      else
      {
        // all other operators we pop off the stack any operator
        // that has a higher priority than the one we have.
        while (!save.empty() && GetOperator(save.top()) > GetOperator(expression[i]))
        {
          // only handle parenthesis once they're closed.
          if (save.top() == '[' && expression[i] != ']')
            break;

          comb.m_postfix.push_back(-GetOperator(save.top()));  // negative denotes operator
          save.pop();
        }
        save.push(expression[i]);
      }
    }
    else
    {
      operand += expression[i];
    }
  }

  if (!operand.empty())
  {
    int op = TranslateSingleString(operand);
    if (op)
      comb.m_postfix.push_back(op);
  }

  // finish up by adding any operators
  while (!save.empty())
  {
    comb.m_postfix.push_back(-GetOperator(save.top()));
    save.pop();
  }

  // test evaluate
  bool test;
  if (!EvaluateBooleanExpression(comb, test, WINDOW_INVALID))
    CLog::Log(LOGERROR, "Error evaluating boolean expression %s", expression.c_str());
  // success - add to our combined values
  m_CombinedValues.push_back(comb);
  return comb.m_id;
}

CStdString CGUIInfoManager::GetLabel(int info, int contextWindow)
{
	CStdString strLabel;

	switch (info)
	{
		case SYSTEM_DATE:
			strLabel = GetDate();
			break;
		case SYSTEM_TIME:
			strLabel = GetTime();
			break;
	}

	return strLabel;
}

// checks the condition and returns it as necessary.  Currently used
// for toggle button controls and visibility of images.
bool CGUIInfoManager::GetBool(int condition)
{
	bool bReturn = false;
	
	if ( condition == SYSTEM_ALWAYS_TRUE)
		bReturn = true;
	else if (condition == SYSTEM_ALWAYS_FALSE)
		bReturn = false;
	else if (condition == PLAYER_SHOWCODEC)
		bReturn = m_playerShowCodec;
	else if (condition >= MULTI_INFO_START && condition <= MULTI_INFO_END)
	{
		// cache return value
		bool result = GetMultiInfoBool(m_multiInfo[condition - MULTI_INFO_START]);
//		if (!item)
//			CacheBool(condition1, contextWindow, result);
		return result;
	}
	else if (g_application.IsPlaying())
	{
		switch (condition)
		{
			case PLAYER_HAS_MEDIA:
				bReturn = true;
				break;
		}
	}

	return bReturn;
}

void CGUIInfoManager::UpdateFPS()
{
	m_frameCounter++;
	float fTimeSpan = (float)(GetTickCount() - m_lastFPSTime);
	
	if (fTimeSpan >= 1000.0f)
	{
		fTimeSpan /= 1000.0f;
		m_fps = m_frameCounter / fTimeSpan;
		m_lastFPSTime = GetTickCount();
		m_frameCounter = 0;
	}
}

int CGUIInfoManager::AddMultiInfo(const GUIInfo &info)
{
	// check to see if we have this info already
	for (unsigned int i = 0; i < m_multiInfo.size(); i++)
	if (m_multiInfo[i] == info)
		return (int)i + MULTI_INFO_START;
	// return the new offset
	m_multiInfo.push_back(info);
	return (int)m_multiInfo.size() + MULTI_INFO_START - 1;
}

/// \brief Examines the multi information sent and returns true or false accordingly.
bool CGUIInfoManager::GetMultiInfoBool(const GUIInfo &info, int contextWindow)
{
	bool bReturn = false;
	int condition = abs(info.m_info);

    switch (condition)
    {
		case CONTROL_HAS_FOCUS:
        {
			CGUIWindow *window = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
			if (window)
				bReturn = (window->GetFocusedControl() == (int)info.GetData1());
        }
        break;
	}

	return (info.m_info < 0) ? !bReturn : bReturn;
}

CStdString CGUIInfoManager::GetDate(bool bNumbersOnly)
{
	CStdString text;
	SYSTEMTIME time;
	GetLocalTime(&time);

	if (bNumbersOnly)
	{
		CStdString strDate;
		if (/*g_guiSettings.GetInt("XBDateTime.DateFormat") == DATETIME_FORMAT_EU*/1)
		  text.Format("%d-%d-%d", time.wDay, time.wMonth, time.wYear);
		else
			text.Format("%d-%d-%d", time.wMonth, time.wDay, time.wYear);
	}
	else
	{
		CStdString day;
		switch (time.wDayOfWeek)
		{
			case 1 : day = g_localizeStrings.Get(11); break;
			case 2 : day = g_localizeStrings.Get(12); break;
			case 3 : day = g_localizeStrings.Get(13); break;
			case 4 : day = g_localizeStrings.Get(14); break;
			case 5 : day = g_localizeStrings.Get(15); break;
			case 6 : day = g_localizeStrings.Get(16); break;
			default: day = g_localizeStrings.Get(17); break;
		}

		CStdString month;
		switch (time.wMonth)
		{
			case 1 : month = g_localizeStrings.Get(21); break;
			case 2 : month = g_localizeStrings.Get(22); break;
			case 3 : month = g_localizeStrings.Get(23); break;
			case 4 : month = g_localizeStrings.Get(24); break;
			case 5 : month = g_localizeStrings.Get(25); break;
			case 6 : month = g_localizeStrings.Get(26); break;
			case 7 : month = g_localizeStrings.Get(27); break;
			case 8 : month = g_localizeStrings.Get(28); break;
			case 9 : month = g_localizeStrings.Get(29); break;
			case 10: month = g_localizeStrings.Get(30); break;
			case 11: month = g_localizeStrings.Get(31); break;
			default: month = g_localizeStrings.Get(32); break;
		}

		if (day.size() && month.size())
		{
			if (/*g_guiSettings.GetInt("XBDateTime.DateFormat") == DATETIME_FORMAT_EU*/1)
				text.Format("%s, %d %s %d", day.c_str(), time.wDay, month.c_str(), time.wYear);
			else
				text.Format("%s, %s %d %d", day.c_str(), month.c_str(), time.wDay, time.wYear);
		}
		else
			text.Format("no date");
	}
	return text;
}

CStdString CGUIInfoManager::GetTime(bool bSeconds)
{
	CStdString text;
	SYSTEMTIME time;
	GetLocalTime(&time);

	INT iHour = time.wHour;

	if (/*g_guiSettings.GetInt("XBDateTime.TimeFormat") == DATETIME_FORMAT_US*/1)
	{
		if (iHour > 11)
		{
			iHour -= (12 * (iHour > 12));
			if (bSeconds)
				text.Format("%2d:%02d:%02d PM", iHour, time.wMinute, time.wSecond);
			else
				text.Format("%2d:%02d PM", iHour, time.wMinute);
		}
		else
		{
			iHour += (12 * (iHour < 1));
			if (bSeconds)
				text.Format("%2d:%02d:%02d AM", iHour, time.wMinute, time.wSecond);
			else
				text.Format("%2d:%02d AM", iHour, time.wMinute);
		}
	}
	else
	{
		if (bSeconds)
			text.Format("%2d:%02d:%02d", iHour, time.wMinute, time.wSecond);
		else
			text.Format("%02d:%02d", iHour, time.wMinute);
	}
	return text;
}

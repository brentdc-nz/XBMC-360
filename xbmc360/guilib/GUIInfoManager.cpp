#include "GUIInfoManager.h"
#include "GUIWindowManager.h"
#include "utils\Log.h"
#include "Application.h"
#include "LocalizeStrings.h"
#include "xbox\XBKernalExports.h"
#include "utils\StringUtils.h"
#include "SkinInfo.h"
#include "utils\URIUtils.h"
#include "Settings.h"
#include "GUISettings.h"
#include "dialogs\GUIDialogProgress.h"
#include "ButtonTranslator.h"
#include "GUIMediaWindow.h"

using namespace std;

CGUIInfoManager g_infoManager;

void GUIInfo::SetInfoFlag(uint32_t flag)
{
	assert(flag >= (1 << 24));
	m_data1 |= flag;
}

uint32_t GUIInfo::GetInfoFlag() const
{
	// We strip out the bottom 24 bits, where we keep data
	// and return the flag only
	return m_data1 & 0xff000000;
}

uint32_t GUIInfo::GetData1() const
{
	// We strip out the top 8 bits, where we keep flags
	// and return the unflagged data
	return m_data1 & ((1 << 24) -1);
}

int GUIInfo::GetData2() const
{
	return m_data2;
}

using namespace INFO;

CGUIInfoManager::CGUIInfoManager(void)
{
	m_nextWindowID = WINDOW_INVALID;
	m_prevWindowID = WINDOW_INVALID;
	m_stringParameters.push_back("__ZZZZ__"); // To offset the string parameters by 1 to assure that all entries are non-zero
	m_frameCounter = 0;
	m_lastFPSTime = 0;
	m_updateTime = 0;
}

CGUIInfoManager::~CGUIInfoManager(void)
{
}

unsigned int CGUIInfoManager::Register(const CStdString &expression, int context)
{
  CStdString condition(CGUIInfoLabel::ReplaceLocalize(expression));
  condition.TrimLeft(" \t\r\n");
  condition.TrimRight(" \t\r\n");

  if (condition.IsEmpty())
    return 0;

  CSingleLock lock(m_critInfo);
  // do we have the boolean expression already registered?
  InfoBool test(condition, context);
  for (unsigned int i = 0; i < m_bools.size(); ++i)
  {
    if (*m_bools[i] == test)
      return i+1;
  }

  if (condition.find_first_of("|+[]!") != condition.npos)
    m_bools.push_back(new InfoExpression(condition, context));
  else
    m_bools.push_back(new InfoSingle(condition, context));

  return m_bools.size();
}

/*
	TODO: what to do with item-based infobools...
	these crop up:
	1. if condition is between LISTITEM_START and LISTITEM_END
	2. if condition is STRING_IS_EMPTY, STRING_COMPARE, STRING_STR, INTEGER_GREATER_THAN and the
	corresponding label is between LISTITEM_START and LISTITEM_END

	In both cases they shouldn't be in our cache as they depend on items outside of our control atm.

	We only pass a listitem object in for controls inside a listitemlayout, so I think it's probably OK
	to not cache these, as they're "pushed" out anyway.

	The problem is how do we avoid these?  The only thing we have to go on is the expression here, so I
	guess what we have to do is call through via Update.  One thing we don't handle, however, is that the
	majority of conditions (even inside lists) don't depend on the listitem at all.

	Advantage is that we know this at creation time I think, so could perhaps signal it in IsDirty()?
 */
bool CGUIInfoManager::GetBoolValue(unsigned int expression, const CGUIListItem *item)
{
	if (expression && --expression < m_bools.size())
		return m_bools[expression]->Get(m_updateTime, item);
	
	return false;
}

// Translates a string as given by the skin into an int that we use for more
// efficient retrieval of data. Can handle combined strings on the form
// Player.Caching + VideoPlayer.IsFullscreen (Logical and)
// Player.HasVideo | Player.HasAudio (Logical or)
int CGUIInfoManager::TranslateString(const CStdString &condition)
{
	// Translate $LOCALIZE as required
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
	// Just single command.
	return TranslateSingleString(strCondition);
}

bool CGUIInfoManager::EvaluateBool(const CStdString &expression, int contextWindow)
{
	bool result = false;
/*	unsigned int info = Register(expression, contextWindow);

	if (info)
		result = GetBoolValue(info);*/

	return result;
}

// Translates a string as given by the skin into an int that we e
// use for morefficient retrieval of data
int CGUIInfoManager::TranslateSingleString(const CStdString &strCondition)
{
	// Trim whitespace, and convert to lowercase
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

	// Translate conditions...
	if(strTest.Equals("false") || strTest.Equals("no") || strTest.Equals("off")) 
		ret = SYSTEM_ALWAYS_FALSE;
	else if(strTest.Equals("true") || strTest.Equals("yes") || strTest.Equals("on")) 
		ret = SYSTEM_ALWAYS_TRUE;

	else if(strCategory.Equals("system"))
	{
		if(strTest.Equals("system.date")) ret = SYSTEM_DATE;
		else if(strTest.Equals("system.time")) ret = SYSTEM_TIME;
		else if(strTest.Equals("system.fps")) ret = SYSTEM_FPS;
		else if(strTest.Equals("system.cputemperature")) ret = SYSTEM_CPU_TEMPERATURE;
		else if(strTest.Equals("system.gputemperature")) ret = SYSTEM_GPU_TEMPERATURE;
		else if(strTest.Equals("system.memory(free)") || strTest.Equals("system.freememory")) ret = SYSTEM_FREE_MEMORY;
		else if (strTest.Equals("system.progressbar")) ret = SYSTEM_PROGRESS_BAR;
	}
	else if (strCategory.Equals("network"))
	{
		if (strTest.Equals("network.ipaddress")) ret = NETWORK_IP_ADDRESS;
		if (strTest.Equals("network.isdhcp")) ret = NETWORK_IS_DHCP;
		if (strTest.Equals("network.linkstate")) ret = NETWORK_LINK_STATE;
		if (strTest.Equals("network.macaddress")) ret = NETWORK_MAC_ADDRESS;
		if (strTest.Equals("network.subnetaddress")) ret = NETWORK_SUBNET_ADDRESS;
		if (strTest.Equals("network.gatewayaddress")) ret = NETWORK_GATEWAY_ADDRESS;
		if (strTest.Equals("network.dns1address")) ret = NETWORK_DNS1_ADDRESS;
		if (strTest.Equals("network.dns2address")) ret = NETWORK_DNS2_ADDRESS;
		if (strTest.Equals("network.dhcpaddress")) ret = NETWORK_DHCP_ADDRESS;
	}
	else if(strCategory.Equals("player"))
	{
		if(strTest.Equals("player.hasmedia")) ret = PLAYER_HAS_MEDIA;
		else if(strTest.Equals("player.time")) ret = PLAYER_TIME;
		else if(strTest.Equals("player.timeremaining")) ret = PLAYER_TIME_REMAINING;
		else if(strTest.Equals("player.duration")) ret = PLAYER_DURATION;
		else if(strTest.Equals("player.progress")) ret = PLAYER_PROGRESS;
		else if (strTest.Equals("player.seeking")) ret = PLAYER_SEEKING;
	}
	else if (strTest.Left(17).Equals("control.hasfocus("))
	{
		int controlID = atoi(strTest.Mid(17, strTest.GetLength() - 18).c_str());
		if (controlID)
			return AddMultiInfo(GUIInfo(bNegate ? -CONTROL_HAS_FOCUS : CONTROL_HAS_FOCUS, controlID, 0));
	}
	else if (strTest.Left(13).Equals("controlgroup("))
	{
		int groupID = atoi(strTest.Mid(13).c_str());
		int controlID = 0;
		int controlPos = strTest.Find(".hasfocus(");
		
		if (controlPos > 0)
			controlID = atoi(strTest.Mid(controlPos + 10).c_str());
		
		if (groupID)
		{
			return AddMultiInfo(GUIInfo(bNegate ? -CONTROL_GROUP_HAS_FOCUS : CONTROL_GROUP_HAS_FOCUS, groupID, controlID));
		}
	}
	else if (strCategory.Left(8).Equals("listitem"))
	{
		int offset = atoi(strCategory.Mid(9, strCategory.GetLength() - 10));
		ret = TranslateListItem(strTest.Mid(strCategory.GetLength() + 1));
		
		if (offset || ret == LISTITEM_ISSELECTED || ret == LISTITEM_ISPLAYING || ret == LISTITEM_IS_FOLDER)
			return AddMultiInfo(GUIInfo(bNegate ? -ret : ret, 0, offset, INFOFLAG_LISTITEM_WRAP));
	}
	else if (strCategory.Equals("skin"))
	{
		if (strTest.Equals("skin.currenttheme"))
			ret = SKIN_THEME;
		else if (strTest.Equals("skin.currentcolourtheme"))
			ret = SKIN_COLOUR_THEME;
		else if (strTest.Left(12).Equals("skin.string("))
		{
			int pos = strTest.Find(",");

			if (pos >= 0)
			{
				int skinOffset = g_settings.TranslateSkinString(strTest.Mid(12, pos - 12));
				int compareString = ConditionalStringParameter(strTest.Mid(pos + 1, strTest.GetLength() - (pos + 2)));
				return AddMultiInfo(GUIInfo(bNegate ? -SKIN_STRING : SKIN_STRING, skinOffset, compareString));
			}
			
			int skinOffset = g_settings.TranslateSkinString(strTest.Mid(12, strTest.GetLength() - 13));
			return AddMultiInfo(GUIInfo(bNegate ? -SKIN_STRING : SKIN_STRING, skinOffset));
		}
		else if (strTest.Left(16).Equals("skin.hassetting("))
		{
			int skinOffset = g_settings.TranslateSkinBool(strTest.Mid(16, strTest.GetLength() - 17));
			return AddMultiInfo(GUIInfo(bNegate ? -SKIN_BOOL : SKIN_BOOL, skinOffset));
		}
		else if (strTest.Left(14).Equals("skin.hastheme("))
			ret = SKIN_HAS_THEME_START + ConditionalStringParameter(strTest.Mid(14, strTest.GetLength() -  15));
	}
	else if (strCategory.Left(6).Equals("window"))
	{
		CStdString info = strTest.Mid(strCategory.GetLength() + 1);
		// Special case for window.xml parameter, fails above
		if (info.Left(5).Equals("xml)."))
			info = info.Mid(5, info.GetLength() + 1);
		if (info.Left(9).Equals("property("))
		{
			int winID = 0;
			if (strTest.Left(7).Equals("window("))
			{
				CStdString window(strTest.Mid(7, strTest.Find(")", 7) - 7).ToLower());
				winID = CButtonTranslator::TranslateWindow(window);
			}
			if (winID != WINDOW_INVALID)
			{
				int compareString = ConditionalStringParameter(info.Mid(9, info.GetLength() - 10));
				return AddMultiInfo(GUIInfo(WINDOW_PROPERTY, winID, compareString));
			}
		}
		else if (info.Left(9).Equals("isactive("))
		{
			CStdString window(strTest.Mid(16, strTest.GetLength() - 17).ToLower());
			if (window.Find("xml") >= 0)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_IS_ACTIVE : WINDOW_IS_ACTIVE, 0, ConditionalStringParameter(window)));
			int winID = CButtonTranslator::TranslateWindow(window);
			if (winID != WINDOW_INVALID)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_IS_ACTIVE : WINDOW_IS_ACTIVE, winID, 0));
		}
		else if (info.Left(7).Equals("ismedia"))
			return WINDOW_IS_MEDIA;
		else if (info.Left(10).Equals("istopmost("))
		{
			CStdString window(strTest.Mid(17, strTest.GetLength() - 18).ToLower());
			if (window.Find("xml") >= 0)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_IS_TOPMOST : WINDOW_IS_TOPMOST, 0, ConditionalStringParameter(window)));
			int winID = CButtonTranslator::TranslateWindow(window);
			if (winID != WINDOW_INVALID)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_IS_TOPMOST : WINDOW_IS_TOPMOST, winID, 0));
		}
		else if (info.Left(10).Equals("isvisible("))
		{
			CStdString window(strTest.Mid(17, strTest.GetLength() - 18).ToLower());
			if (window.Find("xml") >= 0)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_IS_VISIBLE : WINDOW_IS_VISIBLE, 0, ConditionalStringParameter(window)));
			int winID = CButtonTranslator::TranslateWindow(window);
			if (winID != WINDOW_INVALID)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_IS_VISIBLE : WINDOW_IS_VISIBLE, winID, 0));
		}
		else if (info.Left(9).Equals("previous("))
		{
			CStdString window(strTest.Mid(16, strTest.GetLength() - 17).ToLower());
			if (window.Find("xml") >= 0)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_PREVIOUS : WINDOW_PREVIOUS, 0, ConditionalStringParameter(window)));
			int winID = CButtonTranslator::TranslateWindow(window);
			if (winID != WINDOW_INVALID)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_PREVIOUS : WINDOW_PREVIOUS, winID, 0));
		}
		else if (info.Left(5).Equals("next("))
		{
			CStdString window(strTest.Mid(12, strTest.GetLength() - 13).ToLower());
			if (window.Find("xml") >= 0)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_NEXT : WINDOW_NEXT, 0, ConditionalStringParameter(window)));
			int winID = CButtonTranslator::TranslateWindow(window);
			if (winID != WINDOW_INVALID)
				return AddMultiInfo(GUIInfo(bNegate ? -WINDOW_NEXT : WINDOW_NEXT, winID, 0));
		}
	}

	return bNegate ? -ret : ret;
}

int CGUIInfoManager::RegisterSkinVariableString(const CSkinVariableString* info)
{
	if (!info)
		return 0;

	CSingleLock lock(m_critInfo);
	m_skinVariableStrings.push_back(*info);

	delete info;
	return CONDITIONAL_LABEL_START + m_skinVariableStrings.size() - 1;
}

int CGUIInfoManager::TranslateSkinVariableString(const CStdString& name, int context)
{
	for (vector<CSkinVariableString>::const_iterator it = m_skinVariableStrings.begin();
		it != m_skinVariableStrings.end(); ++it)
	{
		if (it->GetName().Equals(name) && it->GetContext() == context)
			return it - m_skinVariableStrings.begin() + CONDITIONAL_LABEL_START;
	}
	return 0;
}

CStdString CGUIInfoManager::GetSkinVariableString(int info,
                                                  bool preferImage /*= false*/,
                                                  const CGUIListItem *item /*= NULL*/)
{
	info -= CONDITIONAL_LABEL_START;
	
	if (info >= 0 && info < (int)m_skinVariableStrings.size())
		return m_skinVariableStrings[info].GetValue(preferImage, item);

	return "";
}

int CGUIInfoManager::GetOperator(const char ch)
{
	if(ch == '[')
		return 5;
	else if(ch == ']')
		return 4;
	else if(ch == '!')
		return OPERATOR_NOT;
	else if(ch == '+')
		return OPERATOR_AND;
	else if(ch == '|')
		return OPERATOR_OR;
	else
		return 0;
}

bool CGUIInfoManager::EvaluateBooleanExpression(const CCombinedValue &expression, bool &result, int contextWindow)
{
	// Stack to save our bool state as we go
	stack<bool> save;

	for (list<int>::const_iterator it = expression.m_postfix.begin(); it != expression.m_postfix.end(); ++it)
	{
		int expr = *it;
		if (expr == -OPERATOR_NOT)
		{
			// NOT the top item on the stack
			if (save.size() < 1) return false;
			bool expr = save.top();
			save.pop();
			save.push(!expr);
		}
		else if (expr == -OPERATOR_AND)
		{
			// AND the top two items on the stack
			if (save.size() < 2) return false;
			bool right = save.top(); save.pop();
			bool left = save.top(); save.pop();
			save.push(left && right);
		}
		else if (expr == -OPERATOR_OR)
		{ 
			// OR the top two items on the stack
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

	// Operator stack
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

	// Success - add to our combined values
	m_CombinedValues.push_back(comb);

	return comb.m_id;
}

int CGUIInfoManager::ConditionalStringParameter(const CStdString &parameter, bool caseSensitive /*= false*/)
{
	// Check to see if we have this parameter already
	for (unsigned int i = 0; i < m_stringParameters.size(); i++)
		if (parameter.Equals(m_stringParameters[i], caseSensitive))
			return (int)i;

	// Return the new offset
	m_stringParameters.push_back(parameter);
	return (int)m_stringParameters.size() - 1;
}

CStdString CGUIInfoManager::GetItemLabel(const CFileItem *item, int info)
{
	if (!item) return "";

	if (info >= CONDITIONAL_LABEL_START && info <= CONDITIONAL_LABEL_END)
		return GetSkinVariableString(info, false, item);

	if (info >= LISTITEM_PROPERTY_START && info - LISTITEM_PROPERTY_START < (int)m_listitemProperties.size())
	{
		// Grab the property  
		CStdString property = m_listitemProperties[info - LISTITEM_PROPERTY_START];
		return item->GetProperty(property);
	}

	switch (info)
	{
		case LISTITEM_LABEL:
			return item->GetLabel();
		case LISTITEM_LABEL2:
			return item->GetLabel2();
		case LISTITEM_ICON:
		{
			CStdString strThumb = item->GetThumbnailImage();
			if(!strThumb.IsEmpty() && !g_TextureManager.CanLoad(strThumb))
				strThumb = "";

			if(strThumb.IsEmpty() && !item->GetIconImage().IsEmpty())
			{
				strThumb = item->GetIconImage();
				if (g_SkinInfo.GetVersion() <= 2.10)
					strThumb.Insert(strThumb.Find("."), "Big");
			}
			return strThumb;
		}
		case LISTITEM_THUMB:
			return item->GetThumbnailImage();

		// TODO - Many missign !!!
	}

	return "";
}

CStdString CGUIInfoManager::GetItemImage(const CFileItem *item, int info)
{
	if (info >= CONDITIONAL_LABEL_START && info <= CONDITIONAL_LABEL_END)
		return GetSkinVariableString(info, true, item);

	return GetItemLabel(item, info);
}

CStdString CGUIInfoManager::GetLabel(int info, int contextWindow)
{
	CStdString strLabel;

	switch (info)
	{
		// System Section
		case SYSTEM_DATE:
			strLabel = GetDate();
			break;
		case SYSTEM_TIME:
			strLabel = GetTime();
			break;
		 case SYSTEM_FPS:
			strLabel.Format("%02.2f", m_fps);
			break;
		case SYSTEM_CPU_TEMPERATURE:
		case SYSTEM_GPU_TEMPERATURE:
			return GetSystemHeatInfo(info);
			break;
		case SYSTEM_FREE_MEMORY:
		{
			MEMORYSTATUS stat;
			GlobalMemoryStatus(&stat);
			strLabel.Format("%iMB", stat.dwAvailPhys /MB);
		}
		break;
		case SYSTEM_PROGRESS_BAR:
		{
			int percent = GetInt(SYSTEM_PROGRESS_BAR);
			if (percent)
				strLabel.Format("%i", percent);
		}
		break;

		// Skin theme
		case SKIN_THEME:
			if (g_guiSettings.GetString("lookandfeel.skintheme").Equals("skindefault"))
				strLabel = "skindefault";
			else
				strLabel = g_guiSettings.GetString("lookandfeel.skintheme");
		break;
		case SKIN_COLOUR_THEME:
			if (g_guiSettings.GetString("lookandfeel.skincolors").Equals("skindefault"))
				strLabel = "skindefault";
			else
				strLabel = g_guiSettings.GetString("lookandfeel.skincolors");
		break;

		// Network section
		case NETWORK_IP_ADDRESS:
		{
			return g_application.getNetwork().m_networkinfo.ip;
		}
		break;
		case NETWORK_SUBNET_ADDRESS:
		{
			return g_application.getNetwork().m_networkinfo.subnet;
		}
		break;
		case NETWORK_GATEWAY_ADDRESS:
		{
			return g_application.getNetwork().m_networkinfo.gateway;
		}
		break;
		case NETWORK_DNS1_ADDRESS:
		{
			return g_application.getNetwork().m_networkinfo.DNS1;
		}
		break;
		case NETWORK_DNS2_ADDRESS:
		{
			return g_application.getNetwork().m_networkinfo.DNS2;
		}
		break;
		case NETWORK_DHCP_ADDRESS:
		{
			return g_application.getNetwork().m_networkinfo.dhcpserver;
		}
		break;
		case NETWORK_IS_DHCP:
		{
			if(g_application.getNetwork().m_networkinfo.DHCP)
				return g_localizeStrings.Get(148); // Is DHCP IP
			else
				return g_localizeStrings.Get(147); // Is fixed IP
		}
		break;
		case NETWORK_LINK_STATE:
		{
			DWORD dwnetstatus = XNetGetEthernetLinkStatus();
			CStdString linkStatus;

			if (dwnetstatus & XNET_ETHERNET_LINK_ACTIVE)
			{
				if (dwnetstatus & XNET_ETHERNET_LINK_100MBPS)
					linkStatus += "100mbps ";
				if (dwnetstatus & XNET_ETHERNET_LINK_10MBPS)
					linkStatus += "10mbps ";
				if (dwnetstatus & XNET_ETHERNET_LINK_FULL_DUPLEX)
					linkStatus += g_localizeStrings.Get(153);
				if (dwnetstatus & XNET_ETHERNET_LINK_HALF_DUPLEX)
					linkStatus += g_localizeStrings.Get(152);
			}
			else
				linkStatus += g_localizeStrings.Get(159);
			return linkStatus;
		}
		break;

		// Player Section	
		case PLAYER_TIME:
			strLabel = GetCurrentPlayTime();
			break;
		case PLAYER_TIME_REMAINING:
			strLabel = GetCurrentPlayTimeRemaining();
			break;
		case PLAYER_DURATION:
			if (g_application.IsPlaying())
				strLabel = GetDuration();
			break;
	}

	return strLabel;
}

// Obtains the filename of the image to show from whichever subsystem is needed
CStdString CGUIInfoManager::GetImage(int info, int contextWindow)
{
	if (info >= CONDITIONAL_LABEL_START && info <= CONDITIONAL_LABEL_END)
		return GetSkinVariableString(info, true);

	if (info >= MULTI_INFO_START && info <= MULTI_INFO_END)
	{
		return GetMultiInfoLabel(m_multiInfo[info - MULTI_INFO_START], contextWindow);
	}
	else if (info == LISTITEM_THUMB || info == LISTITEM_ICON || info == LISTITEM_ACTUAL_ICON ||
          info == LISTITEM_OVERLAY || info == LISTITEM_RATING || info == LISTITEM_STAR_RATING)
	{
		CGUIWindow *window = GetWindowWithCondition(contextWindow, WINDOW_CONDITION_HAS_LIST_ITEMS);
		if (window)
		{
			CFileItemPtr item = window->GetCurrentListItem();
			if (item)
				return GetItemImage(item.get(), info);
		}
	}
	return GetLabel(info, contextWindow);
}

// Checks the condition and returns it as necessary.  Currently used
// for toggle button controls and visibility of images
bool CGUIInfoManager::GetBool(int condition1, int contextWindow, const CGUIListItem *item)
{
	bool bReturn = false;
	
	int condition = abs(condition1);

	if(condition >= COMBINED_VALUES_START && (condition - COMBINED_VALUES_START) < (int)(m_CombinedValues.size()) )
	{
		const CCombinedValue &comb = m_CombinedValues[condition - COMBINED_VALUES_START];

		if (!EvaluateBooleanExpression(comb, bReturn, contextWindow/*, item*/)) //TODO
			bReturn = false;
	}
  	else if(condition == SYSTEM_ALWAYS_TRUE)
		bReturn = true;
	else if(condition == SYSTEM_ALWAYS_FALSE)
		bReturn = false;
	else if(condition == PLAYER_SHOWCODEC)
		bReturn = m_playerShowCodec;
	else if (condition >= SKIN_HAS_THEME_START && condition <= SKIN_HAS_THEME_END)
	{
		// Note that the code used here could probably be extended to general
		// settings conditions (parameter would need to store both the setting name an
		// the and the comparison string)
		CStdString theme = g_guiSettings.GetString("lookandfeel.skintheme");
		theme.ToLower();
		URIUtils::RemoveExtension(theme);
		bReturn = theme.Equals(m_stringParameters[condition - SKIN_HAS_THEME_START]);
	}
	else if(condition >= MULTI_INFO_START && condition <= MULTI_INFO_END)
	{
		// Cache return value
		bool result = GetMultiInfoBool(m_multiInfo[condition - MULTI_INFO_START], contextWindow/*, item*/);
//		if(!item)
//			CacheBool(condition1, contextWindow, result); //TODO
		return result;
	}
	else if(g_application.IsPlaying())
	{
		switch(condition)
		{
			case PLAYER_HAS_MEDIA:
				bReturn = true;
			break;
		}
	}

	// Cache return value
	if(condition1 < 0) bReturn = !bReturn;

	return bReturn;
}

// Tries to get a integer value for use in progressbars/sliders and such
int CGUIInfoManager::GetInt(int info, int contextWindow) const
{
	switch(info)
	{
		case PLAYER_PROGRESS:
		{
			if(g_application.IsPlaying() && g_application.m_pPlayer)
			{
				switch(info)
				{
					case PLAYER_PROGRESS:
						return (int)(g_application.GetPercentage());
				}
			}
		}
		break;
		case SYSTEM_PROGRESS_BAR:
		{
			CGUIDialogProgress *bar = (CGUIDialogProgress *)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
			if (bar && bar->IsDialogRunning())
				return bar->GetPercentage();
		}
	}
	return 0; // Not found
}

void CGUIInfoManager::Clear()
{
	CSingleLock lock(m_critInfo);

	for (unsigned int i = 0; i < m_bools.size(); ++i)
		delete m_bools[i];
	
	m_bools.clear();
	
	m_skinVariableStrings.clear();
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

bool CGUIInfoManager::CheckWindowCondition(CGUIWindow *window, int condition) const //TODO
{
	// Check if it satisfies our condition
	if (!window) return false;
/*
	if ((condition & WINDOW_CONDITION_HAS_LIST_ITEMS) && !window->HasListItems())
		return false;

	if ((condition & WINDOW_CONDITION_IS_MEDIA_WINDOW) && !window->IsMediaWindow())
		return false;
*/	
	return true;
}

CGUIWindow *CGUIInfoManager::GetWindowWithCondition(int contextWindow, int condition) const
{
	CGUIWindow *window = g_windowManager.GetWindow(contextWindow);
	if (CheckWindowCondition(window, condition))
		return window;

	// Try topmost dialog
	window = g_windowManager.GetWindow(g_windowManager.GetTopMostModalDialogID());
	if (CheckWindowCondition(window, condition))
		return window;

	// Try active window
	window = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
	if (CheckWindowCondition(window, condition))
		return window;

	return NULL;
}


int CGUIInfoManager::AddMultiInfo(const GUIInfo &info)
{
	// Check to see if we have this info already
	for (unsigned int i = 0; i < m_multiInfo.size(); i++)
	
	if (m_multiInfo[i] == info)
		return (int)i + MULTI_INFO_START;
	
	// Return the new offset
	m_multiInfo.push_back(info);

	return (int)m_multiInfo.size() + MULTI_INFO_START - 1;
}

// Examines the multi information sent and returns the string as appropriate
CStdString CGUIInfoManager::GetMultiInfoLabel(const GUIInfo &info, int contextWindow)
{
	if (info.m_info == SKIN_STRING)
	{
		return g_settings.GetSkinString(info.GetData1());
	}
	else if (info.m_info == SKIN_BOOL)
	{
		bool bInfo = g_settings.GetSkinBool(info.GetData1());
		if (bInfo)
			return g_localizeStrings.Get(20122);
	}
	else if (info.m_info == WINDOW_PROPERTY)
	{
		CGUIWindow *window = NULL;
		if (info.GetData1())
		{
			// Window specified
			window = g_windowManager.GetWindow(info.GetData1());//GetWindowWithCondition(contextWindow, 0);
		}
		else
		{
			// No window specified - assume active
			window = GetWindowWithCondition(contextWindow, 0);
		}

		if (window)
			return window->GetProperty(m_stringParameters[info.GetData2()]);
	}

	// TODO - Add more!

	return CStringUtils::EmptyString;
}

bool CGUIInfoManager::GetItemBool(const CGUIListItem *item, int condition) const
{
	return false; // TODO
}

// Examines the multi information sent and returns true or false accordingly.
bool CGUIInfoManager::GetMultiInfoBool(const GUIInfo &info, int contextWindow)
{
	bool bReturn = false;
	int condition = abs(info.m_info);

	if (condition >= LISTITEM_START && condition <= LISTITEM_END)
	{
		// TODO: We currently don't use the item that is passed in to here, as these
		//       conditions only come from Container(id).ListItem(offset).* at this point.
		CGUIListItemPtr item;
		CGUIWindow *window = NULL;
		
		int data1 = info.GetData1();
		if (!data1) // No container specified, so we lookup the current view container
		{
			window = GetWindowWithCondition(contextWindow, WINDOW_CONDITION_HAS_LIST_ITEMS);
			if (window && window->IsMediaWindow())
				data1 = ((CGUIMediaWindow*)(window))->GetViewContainerID();
		}

		if (!window) // If we don't have a window already (from lookup above), get one
			window = GetWindowWithCondition(contextWindow, 0);

		if (window)
		{
			const CGUIControl *control = window->GetControl(data1);
			if (control && control->IsContainer())
				item = ((CGUIBaseContainer *)control)->GetListItem(info.GetData2(), info.GetInfoFlag());
		}

		if (item) // If we got a valid item, do the lookup
			bReturn = GetItemBool(item.get(), condition); // Image prioritizes images over labels (in the case of music item ratings for instance)
	}
	else
	{
		switch (condition)
		{
			case SKIN_BOOL:
			{
				bReturn = g_settings.GetSkinBool(info.GetData1());
			}
			break;
			case SKIN_STRING:
			{
				if (info.GetData2())
					bReturn = g_settings.GetSkinString(info.GetData1()).Equals(m_stringParameters[info.GetData2()]);
				else
					bReturn = !g_settings.GetSkinString(info.GetData1()).IsEmpty();
			}
			break;
			case CONTROL_HAS_FOCUS:
			{
				CGUIWindow *window = NULL;

				if(g_windowManager.HasDialogOnScreen()) // BRENT : Why is this needed in our code. somthing missing?
					window = g_windowManager.GetWindow(g_windowManager.GetTopMostModalDialogID());
				else
					window = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());

				if (window)
					bReturn = (window->GetFocusedControlID() == (int)info.GetData1());
			}
			break;
			case CONTROL_GROUP_HAS_FOCUS:
			{
				CGUIWindow *window = GetWindowWithCondition(contextWindow, 0);
				if (window)
					bReturn = window->ControlGroupHasFocus(info.GetData1(), info.GetData2());
			}
			break;
			case WINDOW_NEXT:
			if (info.GetData1())
				bReturn = ((int)info.GetData1() == m_nextWindowID);
			else
			{
				CGUIWindow *window = g_windowManager.GetWindow(m_nextWindowID);
				if (window && URIUtils::GetFileName(window->GetProperty("xmlfile")).Equals(m_stringParameters[info.GetData2()]))
					bReturn = true;
			}
			break;
			case WINDOW_PREVIOUS:
			if (info.GetData1())
				bReturn = ((int)info.GetData1() == m_prevWindowID);
			else
			{
				CGUIWindow *window = g_windowManager.GetWindow(m_prevWindowID);
				if (window && URIUtils::GetFileName(window->GetProperty("xmlfile")).Equals(m_stringParameters[info.GetData2()]))
					bReturn = true;
			}
			break;
			case WINDOW_IS_VISIBLE:
			if (info.GetData1())
				bReturn = g_windowManager.IsWindowVisible(info.GetData1());
			else
				bReturn = g_windowManager.IsWindowVisible(m_stringParameters[info.GetData2()]);
			break;
			case WINDOW_IS_TOPMOST:
			if (info.GetData1())
				bReturn = g_windowManager.IsWindowTopMost(info.GetData1());
			else
				bReturn = g_windowManager.IsWindowTopMost(m_stringParameters[info.GetData2()]);
			break;
			case WINDOW_IS_ACTIVE:
			if (info.GetData1())
				bReturn = g_windowManager.IsWindowActive(info.GetData1());
			else
				bReturn = g_windowManager.IsWindowActive(m_stringParameters[info.GetData2()]);
			break;
		}
	}

	return (info.m_info < 0) ? !bReturn : bReturn;
}



int CGUIInfoManager::TranslateListItem(const CStdString &info)
{
	if (info.Equals("thumb")) return LISTITEM_THUMB;
	else if (info.Equals("icon")) return LISTITEM_ICON;
	else if (info.Equals("actualicon")) return LISTITEM_ACTUAL_ICON;
	else if (info.Equals("overlay")) return LISTITEM_OVERLAY;
	else if (info.Equals("label")) return LISTITEM_LABEL;
	else if (info.Equals("label2")) return LISTITEM_LABEL2;
	else if (info.Equals("title")) return LISTITEM_TITLE;
	else if (info.Equals("tracknumber")) return LISTITEM_TRACKNUMBER;
	else if (info.Equals("artist")) return LISTITEM_ARTIST;
	else if (info.Equals("album")) return LISTITEM_ALBUM;
	else if (info.Equals("albumartist")) return LISTITEM_ALBUM_ARTIST;
	else if (info.Equals("year")) return LISTITEM_YEAR;
	else if (info.Equals("genre")) return LISTITEM_GENRE;
	else if (info.Equals("director")) return LISTITEM_DIRECTOR;
	else if (info.Equals("filename")) return LISTITEM_FILENAME;
	else if (info.Equals("filenameandpath")) return LISTITEM_FILENAME_AND_PATH;
	else if (info.Equals("fileextension")) return LISTITEM_FILE_EXTENSION;
	else if (info.Equals("date")) return LISTITEM_DATE;
	else if (info.Equals("size")) return LISTITEM_SIZE;
	else if (info.Equals("rating")) return LISTITEM_RATING;
	else if (info.Equals("ratingandvotes")) return LISTITEM_RATING_AND_VOTES;
	else if (info.Equals("programcount")) return LISTITEM_PROGRAM_COUNT;
	else if (info.Equals("duration")) return LISTITEM_DURATION;
	else if (info.Equals("isselected")) return LISTITEM_ISSELECTED;
	else if (info.Equals("isplaying")) return LISTITEM_ISPLAYING;
	else if (info.Equals("plot")) return LISTITEM_PLOT;
	else if (info.Equals("plotoutline")) return LISTITEM_PLOT_OUTLINE;
	else if (info.Equals("episode")) return LISTITEM_EPISODE;
	else if (info.Equals("season")) return LISTITEM_SEASON;
	else if (info.Equals("tvshowtitle")) return LISTITEM_TVSHOW;
	else if (info.Equals("premiered")) return LISTITEM_PREMIERED;
	else if (info.Equals("comment")) return LISTITEM_COMMENT;
	else if (info.Equals("path")) return LISTITEM_PATH;
	else if (info.Equals("foldername")) return LISTITEM_FOLDERNAME;
	else if (info.Equals("folderpath")) return LISTITEM_FOLDERPATH;
	else if (info.Equals("picturepath")) return LISTITEM_PICTURE_PATH;
	else if (info.Equals("pictureresolution")) return LISTITEM_PICTURE_RESOLUTION;
	else if (info.Equals("picturedatetime")) return LISTITEM_PICTURE_DATETIME;
	else if (info.Equals("studio")) return LISTITEM_STUDIO;
	else if (info.Equals("country")) return LISTITEM_COUNTRY;
	else if (info.Equals("mpaa")) return LISTITEM_MPAA;
	else if (info.Equals("cast")) return LISTITEM_CAST;
	else if (info.Equals("castandrole")) return LISTITEM_CAST_AND_ROLE;
	else if (info.Equals("writer")) return LISTITEM_WRITER;
	else if (info.Equals("tagline")) return LISTITEM_TAGLINE;
	else if (info.Equals("top250")) return LISTITEM_TOP250;
	else if (info.Equals("trailer")) return LISTITEM_TRAILER;
	else if (info.Equals("starrating")) return LISTITEM_STAR_RATING;
	else if (info.Equals("sortletter")) return LISTITEM_SORT_LETTER;
	else if (info.Equals("videocodec")) return LISTITEM_VIDEO_CODEC;
	else if (info.Equals("videoresolution")) return LISTITEM_VIDEO_RESOLUTION;
	else if (info.Equals("videoaspect")) return LISTITEM_VIDEO_ASPECT;
	else if (info.Equals("audiocodec")) return LISTITEM_AUDIO_CODEC;
	else if (info.Equals("audiochannels")) return LISTITEM_AUDIO_CHANNELS;
	else if (info.Equals("audiolanguage")) return LISTITEM_AUDIO_LANGUAGE;
	else if (info.Equals("subtitlelanguage")) return LISTITEM_SUBTITLE_LANGUAGE;
	else if (info.Equals("isfolder")) return LISTITEM_IS_FOLDER;
	else if (info.Equals("originaltitle")) return LISTITEM_ORIGINALTITLE;
	else if (info.Equals("lastplayed")) return LISTITEM_LASTPLAYED;
	else if (info.Equals("playcount")) return LISTITEM_PLAYCOUNT;
//	else if (info.Equals("lastplayed")) return MUSICPLAYER_LASTPLAYED; // TODO
	else if (info.Equals("discnumber")) return LISTITEM_DISC_NUMBER;
	else if (info.Equals("isresumable")) return LISTITEM_IS_RESUMABLE;
//	else if (info.Left(9).Equals("property(")) return AddListItemProp(info.Mid(9, info.GetLength() - 10)); // TODO

	return 0;
}

CStdString CGUIInfoManager::GetDate(bool bNumbersOnly)
{
	CStdString text;
	SYSTEMTIME time;
	memset(&time, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&time);

	if (bNumbersOnly)
	{
		CStdString strDate;
		if (/*g_guiSettings.GetInt("XBDateTime.DateFormat") == DATETIME_FORMAT_EU*/1) //TODO
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
			if (/*g_guiSettings.GetInt("XBDateTime.DateFormat") == DATETIME_FORMAT_EU*/1) //TODO
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
	memset(&time, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&time);

	INT iHour = time.wHour;

	if (/*g_guiSettings.GetInt("XBDateTime.TimeFormat") == DATETIME_FORMAT_US*/1) //TODO
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

CStdString CGUIInfoManager::GetSystemHeatInfo(int info)
{
	CStdString strTemp;

	switch (info)
	{
		case SYSTEM_CPU_TEMPERATURE:
			if (/*g_guiSettings.GetInt("Weather.TemperatureUnits") == */0 /*DEGREES_F*/) //TODO
				strTemp.Format("%3.0i%cF", (9.0f/5.0f) * CXBKernalExports::QueryTemps(CPU) + 32.0f);
			 else
				strTemp.Format("%2.0i%cC", CXBKernalExports::QueryTemps(CPU), 176);
			break;
		case SYSTEM_GPU_TEMPERATURE:
			if (/*g_guiSettings.GetInt("Weather.TemperatureUnits") == */0 /*DEGREES_F*/) //TODO
				strTemp.Format("%3.0i%cF", (9.0f/5.0f) * CXBKernalExports::QueryTemps(GPU) + 32.0f);
			 else
				strTemp.Format("%2.0i%cC", CXBKernalExports::QueryTemps(GPU), 176);
			break;
	}
	return strTemp;
}

__int64 CGUIInfoManager::GetPlayTime() const
{
	if (g_application.IsPlaying())
	{
		__int64 lPTS = (__int64)(g_application.GetTime() * 1000);
		if (lPTS < 0) lPTS = 0;
		return lPTS;
	}

	return 0;
}

CStdString CGUIInfoManager::GetCurrentPlayTime(TIME_FORMAT format) const
{
	if (format == TIME_FORMAT_GUESS && GetTotalPlayTime() >= 3600)
		format = TIME_FORMAT_HH_MM_SS;

	if (g_application.IsPlayingAudio() || g_application.IsPlayingVideo())
		return CStringUtils::SecondsToTimeString((int)(GetPlayTime()/1000), format);

	return "";
}

int CGUIInfoManager::GetPlayTimeRemaining() const
{
	int iReverse = GetTotalPlayTime() - (int)g_application.GetTime();
	return iReverse > 0 ? iReverse : 0;
}

CStdString CGUIInfoManager::GetCurrentPlayTimeRemaining(TIME_FORMAT format) const
{
	if (format == TIME_FORMAT_GUESS && GetTotalPlayTime() >= 3600)
		format = TIME_FORMAT_HH_MM_SS;

	int timeRemaining = GetPlayTimeRemaining();
	
	if (timeRemaining && (g_application.IsPlayingAudio() || g_application.IsPlayingVideo()))
		return CStringUtils::SecondsToTimeString(timeRemaining, format);

	return "";
}

int CGUIInfoManager::GetTotalPlayTime() const
{
	int iTotalTime = (int)g_application.GetTotalTime();

	return iTotalTime > 0 ? iTotalTime : 0;
}

CStdString CGUIInfoManager::GetDuration(TIME_FORMAT format) const
{
	unsigned int iTotal = (unsigned int)g_application.GetTotalTime();
	
	if (iTotal > 0)
		return CStringUtils::SecondsToTimeString(iTotal, format);

	return "";
}

void CGUIInfoManager::ResetCache() // TODO
{
	CSingleLock lock(m_critInfo);
//	m_boolCache.clear();// TODO
	
	// Reset any animation triggers as well
//	m_containerMoves.clear();// TODO
	m_updateTime++;
}
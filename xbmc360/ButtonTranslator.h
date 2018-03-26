#ifndef BUTTONTRANSLATOR_H
#define BUTTONTRANSLATOR_H

#include "utils\Stdafx.h"
#include "utils\StdString.h"
#include "guilib\tinyxml\tinyxml.h"
#include "guilib\Key.h"
#include <map>

struct CButtonAction
{
	WORD wID;
	CStdString strID; // needed for "XBMC.ActivateWindow()" type actions
};

// class to map from buttons to actions
class CButtonTranslator
{
public:
	CButtonTranslator();
	virtual ~CButtonTranslator();

	bool Load();
	WORD TranslateWindowString(const char *szWindow);
	void GetAction(WORD wWindow, const CKey &key, CAction &action);
	void Clear();

private:
	typedef std::multimap<WORD, CButtonAction> buttonMap; // our button map to fill in
	std::map<WORD, buttonMap> translatorMap; // mapping of windows to button maps

	void MapWindowActions(TiXmlNode *pWindow, WORD wWindowID);
	void MapAction(WORD wButtonCode, const char *szAction, buttonMap &map);
	WORD TranslateGamepadString(const char *szButton);
	bool TranslateActionString(const char *szAction, WORD &wAction);
	WORD GetActionCode(WORD wWindow, const CKey &key, CStdString &strAction);
};

extern CButtonTranslator g_buttonTranslator;

#endif //BUTTONTRANSLATOR_H
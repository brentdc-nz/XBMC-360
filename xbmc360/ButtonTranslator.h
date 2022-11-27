#ifndef BUTTONTRANSLATOR_H
#define BUTTONTRANSLATOR_H

#include "utils\Stdafx.h"
#include "utils\StdString.h"
#include "guilib\tinyxml\tinyxml.h"
#include "guilib\Key.h"
#include <map>

class CAction;

struct CButtonAction
{
	int iID;
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
	 static bool TranslateActionString(const char *szAction, int &action);
	void GetAction(WORD wWindow, const CKey &key, CAction &action);

	// Translate between a window name and it's id
	// param window name of the window
	// return id of the window, or WINDOW_INVALID if not found
	static int TranslateWindow(const CStdString &window);

	void Clear();

private:
	typedef std::multimap<WORD, CButtonAction> buttonMap; // Our button map to fill in
	std::map<WORD, buttonMap> translatorMap; // Mapping of windows to button maps

	void MapWindowActions(TiXmlNode *pWindow, WORD wWindowID);
	void MapAction(WORD wButtonCode, const char *szAction, buttonMap &map);
	WORD TranslateGamepadString(const char *szButton);
	WORD GetActionCode(WORD wWindow, const CKey &key, CStdString &strAction);
};

extern CButtonTranslator g_buttonTranslator;

#endif //BUTTONTRANSLATOR_H
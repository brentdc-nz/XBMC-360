#ifndef BUTTONTRANSLATOR_H
#define BUTTONTRANSLATOR_H

#include "utils\stdafx.h"
#include "utils\StdString.h"

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

	WORD TranslateWindowString(const char *szWindow);

private:

};

extern CButtonTranslator g_buttonTranslator;

#endif //BUTTONTRANSLATOR_H
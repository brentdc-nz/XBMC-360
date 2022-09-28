#ifndef GUILIB_LOCALIZESTRINGS_H
#define GUILIB_LOCALIZESTRINGS_H

#include "utils\Stdafx.h"
#include "utils\StdString.h"
#include <string>
#include <map>

class CLocalizeStrings
{
public:
	CLocalizeStrings(void);
	virtual ~CLocalizeStrings(void);
	
	bool Load(const CStdString& strPathName, const CStdString& strLanguage);
	const std::string& Get(DWORD dwCode) const;
	void Clear();

protected:
	std::map<DWORD, std::string> m_vecStrings;
	typedef std::map<DWORD, std::string>::const_iterator ivecStrings;
};

extern CLocalizeStrings g_localizeStrings;

#endif //GUILIB_LOCALIZESTRINGS_H
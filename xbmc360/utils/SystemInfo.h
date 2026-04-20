#pragma once

#include "InfoLoader.h"

class CSysInfo : public CInfoLoader
{
public:
	CSysInfo(void);
	virtual ~CSysInfo();

	CStdString GetUserAgent();

protected:
	virtual bool DoWork();
	virtual CStdString TranslateInfo(int info) const;
};

extern CSysInfo g_sysinfo;

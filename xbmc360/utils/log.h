#ifndef H_CLOG
#define H_CLOG

#include <stdio.h>
#include "StdString.h"

#define LOG_LEVEL_NONE         -1 // Nothing at all is logged
#define LOG_LEVEL_NORMAL        0 // Shows notice, error, severe and fatal
#define LOG_LEVEL_DEBUG         1 // Shows all
#define LOG_LEVEL_DEBUG_FREEMEM 2 // Shows all + shows freemem on screen
#define LOG_LEVEL_DEBUG_SAMBA   3 // Shows all + freemem on screen + samba debugging
#define LOG_LEVEL_MAX           LOG_LEVEL_DEBUG_SAMBA

// Ones we use in the code
#define LOGDEBUG   0
#define LOGINFO    1
#define LOGNOTICE  2
#define LOGWARNING 3
#define LOGERROR   4
#define LOGSEVERE  5
#define LOGFATAL   6
#define LOGNONE    7

class CLog
{
private:
	static FILE*      m_fd;
	static int        m_logLevel;
	static int        m_repeatCount;
	static int        m_repeatLogLevel;
	static CStdString m_repeatLine;

public:
	CLog();
	virtual ~CLog(void);

	static void Close();
	static void Log(int loglevel, const char *format, ... );
	static void DebugLog(const char *format, ...);
	static void MemDump(BYTE *pData, int length);
	static void SetLogLevel(int level);
	static int  GetLogLevel();
};

#endif //H_CLOG
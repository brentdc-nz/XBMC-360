#ifndef H_CADVANCEDSETTINGS
#define H_CADVANCEDSETTINGS

#include "utils\StdString.h"
#include "filesystem\File.h"
using XFILE::CFile;

class CAdvancedSettings
{
public:
	CAdvancedSettings();

	bool Load();
	void Clear();

	int m_logLevel;

	bool m_videoUseTimeSeeking;
	int m_videoTimeSeekForwardBig;
	int m_videoTimeSeekBackwardBig;
	int m_videoTimeSeekForward;
	int m_videoTimeSeekBackward;
	int m_videoPercentSeekForwardBig;
	int m_videoPercentSeekBackwardBig;
	int m_videoPercentSeekForward;
	int m_videoPercentSeekBackward;
	CStdString m_videoPPFFmpegDeint;
	CStdString m_videoPPFFmpegPostProc;
	int m_iSkipLoopFilter;
};

extern CAdvancedSettings g_advancedSettings;

#endif //H_CADVANCEDSETTINGS
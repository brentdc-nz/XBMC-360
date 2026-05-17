#ifndef H_CADVANCEDSETTINGS
#define H_CADVANCEDSETTINGS

#include "utils\StdString.h"

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
	int m_videoSmallStepBackSeconds;
	bool m_musicUseTimeSeeking;
	int m_musicTimeSeekForward;
	int m_musicTimeSeekBackward;
	int m_musicTimeSeekForwardBig;
	int m_musicTimeSeekBackwardBig;
	int m_musicPercentSeekForward;
	int m_musicPercentSeekBackward;
	int m_musicPercentSeekForwardBig;
	int m_musicPercentSeekBackwardBig;

	int m_songInfoDuration; // Duration in seconds to show song info in visualisation
	bool m_bNavVKeyboard; // If true we navigate the virtual keyboard using cursor keys

	int m_videoIgnorePercentAtEnd;
	int m_videoIgnoreSecondsAtStart;

	float m_audioPlayCountMinimumPercent;
	float m_videoPlayCountMinimumPercent;

	int m_curlconnecttimeout;
	int m_curllowspeedtime;
	int m_curlretries;

	int m_thumbSize;
	int m_fanartHeight;
	bool m_useddsfanart;

	CStdString m_videoItemSeparator;
	bool m_bVideoLibraryMyMoviesCategoriesToGenres;

	int m_bgInfoLoaderMaxThreads;
};

extern CAdvancedSettings g_advancedSettings;

#endif //H_CADVANCEDSETTINGS
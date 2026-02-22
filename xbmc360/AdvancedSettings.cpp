#include "AdvancedSettings.h"
#include "guilib\tinyxml\tinyxml.h"
#include "filesystem\File.h"
#include "utils\Log.h"
#include "guilib\XMLUtils.h"

using namespace XFILE;

CAdvancedSettings g_advancedSettings;

CAdvancedSettings::CAdvancedSettings()
{
	m_logLevel = LOG_LEVEL_NORMAL;

	m_videoUseTimeSeeking = true;
	m_videoTimeSeekForwardBig = 600;
	m_videoTimeSeekBackwardBig = -600;
	m_videoTimeSeekForward = 30;
	m_videoTimeSeekBackward = -30;
	m_videoPercentSeekForwardBig = 10;
	m_videoPercentSeekBackwardBig = -10;
	m_videoPercentSeekForward = 2;
	m_videoPercentSeekBackward = -2;
	m_videoPPFFmpegDeint = "linblenddeint";
	m_videoPPFFmpegPostProc = "ha:128:7,va,dr";
	m_iSkipLoopFilter = 0;
	m_videoSmallStepBackSeconds = 7;

	// Music seek defaults from Source xbmc4xbox AdvancedSettings.cpp
	m_musicUseTimeSeeking = true;
	m_musicTimeSeekForward = 10;
	m_musicTimeSeekBackward = -10;
	m_musicTimeSeekForwardBig = 60;
	m_musicTimeSeekBackwardBig = -60;
	m_musicPercentSeekForward = 1;
	m_musicPercentSeekBackward = -1;
	m_musicPercentSeekForwardBig = 10;
	m_musicPercentSeekBackwardBig = -10;

	m_songInfoDuration = 10; // 10 seconds default
	m_bNavVKeyboard = false;
}

bool CAdvancedSettings::Load()
{
	// TODO
	return true;
}

void CAdvancedSettings::Clear()
{
}
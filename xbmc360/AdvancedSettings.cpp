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
#include "VideoSettings.h"

CVideoSettings::CVideoSettings()
{
	m_ViewMode = 0;
	m_CustomZoomAmount = 1.0f;
	m_CustomPixelRatio = 1.0f;
	m_AudioDelay = 0.0f;
	m_AudioStream = -1;
	m_SubtitleStream = -1;
	m_SubtitleDelay = 0.0f;
	m_SubtitleOn = true;
	m_SubtitleCached = false;
	m_PostProcess = false;
	m_InterlaceMethod = VS_INTERLACEMETHOD_NONE;
}
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

bool CVideoSettings::operator!=(const CVideoSettings &right) const
{
	if (m_InterlaceMethod != right.m_InterlaceMethod) return true;
	if (m_ViewMode != right.m_ViewMode) return true;
	if (m_CustomZoomAmount != right.m_CustomZoomAmount) return true;
	if (m_CustomPixelRatio != right.m_CustomPixelRatio) return true;
	if (m_AudioStream != right.m_AudioStream) return true;
	if (m_SubtitleStream != right.m_SubtitleStream) return true;
	if (m_SubtitleDelay != right.m_SubtitleDelay) return true;
	if (m_SubtitleOn != right.m_SubtitleOn) return true;
	if (m_SubtitleCached != right.m_SubtitleCached) return true;
	if (m_PostProcess != right.m_PostProcess) return true;
	if (m_AudioDelay != right.m_AudioDelay) return true;
	return false;
}
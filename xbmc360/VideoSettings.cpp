#include "VideoSettings.h"

CVideoSettings::CVideoSettings()
{
	m_AudioDelay = 0.0f;
	m_AudioStream = -1;
	m_PostProcess = false;
	m_InterlaceMethod = VS_INTERLACEMETHOD_NONE;
}
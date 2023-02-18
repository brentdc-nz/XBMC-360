#include "XBVideoConfig.h"

XBVideoConfig g_videoConfig;

XBVideoConfig::XBVideoConfig()
{
	XMemSet(&m_dwVideoFlags, 0, sizeof(XVIDEO_MODE)); 
	XGetVideoMode(&m_dwVideoFlags);
}

XBVideoConfig::~XBVideoConfig()
{
}

bool XBVideoConfig::HasWidescreen() const
{
	if(m_dwVideoFlags.fIsWideScreen)
		return true;

	return false;
}

bool XBVideoConfig::HasLetterbox() const
{
	if(!m_dwVideoFlags.fIsHiDef)
		return true;

	return false;
}

bool XBVideoConfig::Has480p() const
{
	if(m_dwVideoFlags.fIsHiDef)
		return true;

	return false;
}

bool XBVideoConfig::Has720p() const
{
	if(m_dwVideoFlags.fIsHiDef)
		return true;

	return false;
}

bool XBVideoConfig::Has1080p() const
{
	if(m_dwVideoFlags.fIsHiDef)
		return true;

	return false;
}

bool XBVideoConfig::IsValidResolution(RESOLUTION res) const
{
	return true; // TODO: Write the proper checks!
}

RESOLUTION XBVideoConfig::GetBestMode() const
{
	RESOLUTION bestRes = INVALID;
	RESOLUTION resolutions[] = {HDTV_1080p, HDTV_720p, HDTV_480p_16x9, HDTV_480p_4x3, NTSC_16x9, NTSC_4x3, PAL_16x9, PAL_4x3, PAL60_16x9, PAL60_4x3, INVALID};
	UCHAR i = 0;
	
	while (resolutions[i] != INVALID)
	{
		if (IsValidResolution(resolutions[i]))
		{
			bestRes = resolutions[i];
			break;
		}
		i++;
	}

	return bestRes;
}

RESOLUTION XBVideoConfig::GetSafeMode() const
{
  return HDTV_480p_16x9; // TODO
}
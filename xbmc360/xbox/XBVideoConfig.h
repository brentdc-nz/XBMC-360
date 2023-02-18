#ifndef XBVIDEOCONFIG_H
#define XBVIDEOCONFIG_H

#include "guilib\GraphicContext.h"

class XBVideoConfig
{
public:
	XBVideoConfig();
	~XBVideoConfig();

	bool HasWidescreen() const;
	bool HasLetterbox() const;
	bool Has480p() const;
	bool Has720p() const;
	bool Has1080p() const;

	bool IsValidResolution(RESOLUTION res) const;
	RESOLUTION GetBestMode() const;
	RESOLUTION GetSafeMode() const;

private:
	XVIDEO_MODE m_dwVideoFlags;
};

extern XBVideoConfig g_videoConfig;

#endif //XBVIDEOCONFIG_H
#pragma once

#include "DVDOverlay.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

class CDVDOverlayImage : public CDVDOverlay
{
public:
	CDVDOverlayImage() : CDVDOverlay(DVDOVERLAY_TYPE_IMAGE)
	{
		data = NULL;
		palette = NULL;
		linesize = 0;
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		source_width  = 0;
		source_height = 0;
	}

	CDVDOverlayImage(const CDVDOverlayImage& src)
		: CDVDOverlay(src)
	{
		data    = (BYTE*)malloc(src.linesize * src.height);
		memcpy(data, src.data, src.linesize * src.height);

		palette = (uint32_t*)malloc(src.palette_colors * 4);
		memcpy(palette, src.palette, src.palette_colors * 4);

		palette_colors = src.palette_colors;
		linesize       = src.linesize;
		x              = src.x;
		y              = src.y;
		width          = src.width;
		height         = src.height;
		source_width   = src.source_width;
		source_height  = src.source_height;

	}

	~CDVDOverlayImage()
	{
		if(data) free(data);
		if(palette) free(palette);
	}

	BYTE*  data;
	int    linesize;

	uint32_t* palette;
	int    palette_colors;

	int    x;
	int    y;
	int    width;
	int    height;
	int    source_width;
	int    source_height;
};

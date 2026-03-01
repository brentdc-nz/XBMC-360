#include "DVDCodecUtils.h"
#include "..\DVDClock.h"
#include <math.h>

void CDVDCodecUtils::FreePicture(DVDVideoPicture* pPicture)
{
	delete[] pPicture->data[0];
	delete pPicture;
}

bool CDVDCodecUtils::CopyPictureToOverlay(YV12Image* pImage, DVDVideoPicture *pSrc)
{
	BYTE *s = pSrc->data[0];
	BYTE *d = pImage->plane[0];
	int w = pSrc->iWidth;
	int h = pSrc->iHeight;

	for (int y = 0; y < h; y++)
	{
		memcpy(d, s, w);
		s += pSrc->iLineSize[0];
		d += pImage->stride[0];
	}

	s = pSrc->data[1];
	d = pImage->plane[1];
	w = pSrc->iWidth >> 1;
	h = pSrc->iHeight >> 1;

	for (int y = 0; y < h; y++)
	{
		memcpy(d, s, w);
		s += pSrc->iLineSize[1];
		d += pImage->stride[1];
	}

	s = pSrc->data[2];
	d = pImage->plane[2];

	for (int y = 0; y < h; y++)
	{
		memcpy(d, s, w);
		s += pSrc->iLineSize[2];
		d += pImage->stride[2];
	}

	return true;
}

double CDVDCodecUtils::NormalizeFrameduration(double frameduration)
{
	// If the duration is within 20 microseconds of a common duration, use that
	const double durations[] = {DVD_TIME_BASE * 1.001 / 24.0, DVD_TIME_BASE / 24.0, DVD_TIME_BASE / 25.0,
								DVD_TIME_BASE * 1.001 / 30.0, DVD_TIME_BASE / 30.0, DVD_TIME_BASE / 50.0,
								DVD_TIME_BASE * 1.001 / 60.0, DVD_TIME_BASE / 60.0};

	double lowestdiff = DVD_TIME_BASE;
	int    selected   = -1;
	
	for (size_t i = 0; i < sizeof(durations) / sizeof(durations[0]); i++)
	{
		double diff = fabs(frameduration - durations[i]);
		
		if (diff < DVD_MSEC_TO_TIME(0.02) && diff < lowestdiff)
		{
			selected = i;
			lowestdiff = diff;
		}
	}
	
  if (selected != -1)
		return durations[selected];
	else
		return frameduration;
}
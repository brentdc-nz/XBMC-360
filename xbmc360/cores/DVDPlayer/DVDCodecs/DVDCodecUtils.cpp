#include "DVDCodecUtils.h"

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
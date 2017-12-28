#include "DVDCodecUtils.h"

void CDVDCodecUtils::FreePicture(DVDVideoPicture* pPicture)
{
	delete[] pPicture->data[0];
	delete pPicture;
}

bool CDVDCodecUtils::CopyPictureToOverlay(RGB32Image_t* pImage, DVDVideoPicture *pSrc)
{
	for(int y=0; y < (int)pSrc->iHeight; y++)
		memcpy(pImage->plane + y*pImage->stride, pSrc->data[0]+y*pSrc->iLineSize[0], pSrc->iWidth*4);

	return true;
}